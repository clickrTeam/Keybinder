#include "mapper.h"
#include "daemon.h"
#include "event.h"
#include "key_channel.h"
#include "util.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <optional>
#include <profile.h>
#include <vector>

namespace {
// TODO: move into utils or somthing
InputEvent trigger_to_input(const BasicTrigger &trigger) noexcept {
    return std::visit(
        overloaded{
            [&](const KeyPress &kp) {
                return InputEvent{kp.key_code, KeyEventType::Press};
            },
            [&](const KeyRelease &kr) {
                return InputEvent{kr.key_code, KeyEventType::Release};
            },
        },
        trigger);
}
} // namespace

Mapper::Mapper(Profile profile, Daemon &daemon, KeyReceiver key_receiver)
    : daemon(daemon), key_receiver(key_receiver), cur_layer_idx(0),
      cur_state_idx(HOME_STATE_IDX) {
    set_profile(profile);
}

bool Mapper::set_profile(Profile p) {
    std::vector<std::vector<State>> new_states;
    std::vector<QHash<InputEvent, std::vector<Bind>>> new_basic_maps;
    std::lock_guard lock_guard(mtx);

    for (const Layer &layer : p.layers) {
        QHash<InputEvent, std::vector<Bind>> basic_map;
        for (const auto &[trigger, binds] : layer.basic_remappings) {
            InputEvent input_event = trigger_to_input(trigger);
            if (basic_map.contains(input_event)) {
                return false;
            }
            std::vector<Bind> binds_vec(binds.begin(), binds.end());
            basic_map[input_event] = binds_vec;
        }
        new_basic_maps.push_back(basic_map);

        auto states_opt = generate_states(layer);
        if (!states_opt.has_value()) {
            return false;
        }
        new_states.push_back(*states_opt);
    }
    this->states = new_states;
    this->basic_maps = new_basic_maps;
    set_layer_inner(p.default_layer);
    return true;
}

bool Mapper::set_layer(size_t new_layer) {
    std::lock_guard lock_guard(mtx);
    if (new_layer >= this->states.size()) {
        return false;
    }
    set_layer_inner(new_layer);
    return true;
}

// must be called with mtx aquired
void Mapper::set_layer_inner(size_t new_layer) {
    qDebug() << "new layer: " << new_layer;
    cur_layer_idx = new_layer;
    cur_state_idx = HOME_STATE_IDX;
    current_timer.reset();
}

void Mapper::queue_binds(const std::vector<Bind> &binds) {
    uint64_t delay_ms = 0;

    for (const Bind &bind : binds) {
        std::visit(
            overloaded{
                [&](const PressKey &bind) {
                    queue_output(InputEvent{bind.key_code, KeyEventType::Press},
                                 delay_ms);
                },
                [&](const ReleaseKey &bind) {
                    queue_output(
                        InputEvent{bind.key_code, KeyEventType::Release},
                        delay_ms);
                },
                [&](const SwapLayer &bind) { set_layer_inner(bind.new_layer); },
                [&](const Wait &wait) { delay_ms += wait.ms; },
                [&](const RunScript &s) { queue_output(s, delay_ms); },
            },
            bind);
    }
}

void Mapper::queue_output(OutputEvent e, uint64_t delay = 0) {
    uint64_t time_to_emit = current_time_ms() + delay;
    queued_events.push_back(std::make_pair(time_to_emit, e));
}

void Mapper::start() {
    std::optional<std::chrono::milliseconds> timeout;
    while (!stopped) {

        auto key_opt = this->key_receiver.wait_key(timeout);
        // It is important to not aquire the mutex while we are waiting for the
        // key
        std::lock_guard lock_guard(mtx);
        const State &cur_state = states.at(cur_layer_idx).at(cur_state_idx);
        if (current_timer && *current_timer <= current_time_ms() &&
            cur_state.timer_transition) {
            apply_transition(*cur_state.timer_transition);
        }
        if (key_opt) {
            InputEvent e = key_opt.value();
            process_input(e);
            processed_events_count++;
        }
        check_queued_events();

        // TODO: right now the sleep is hardcoded to 10 ms which is probably
        // fine but maybe a differnt value is better
        timeout = current_timer || !queued_events.empty()
                      ? std::optional(std::chrono::milliseconds(10))
                      : std::nullopt;
    }
}

// Must be called wil mtx acquired
void Mapper::apply_transition(const Transition &transition,
                              std::optional<InputEvent> prev_event) {
    cur_state_idx = transition.new_state;
    current_timer =
        transition.timer_ms
            ? std::optional(*transition.timer_ms + current_time_ms())
            : std::nullopt;

    std::vector<InputEvent> events_to_redo;
    for (const StateMachineOutputEvent &output : transition.outputs) {
        std::visit(overloaded{
                       [&](const ProccessInput &proc) {
                           if (proc.event) {
                               events_to_redo.push_back(*proc.event);
                           } else if (prev_event) {
                               events_to_redo.push_back(*prev_event);
                           }
                       },
                       [&](const BasicTranlation &basic) {
                           auto event = basic.event;
                           if (!event && prev_event)
                               event = prev_event;
                           if (event &&
                               basic_maps.at(cur_layer_idx).contains(*event)) {
                               queue_binds(
                                   basic_maps.at(cur_layer_idx)[*event]);
                           } else if (event) {
                               queue_output(*event);
                           }
                       },
                       [&](const Bind &bind) { queue_binds({bind}); },
                   },
                   output);
    }

    for (InputEvent e : events_to_redo) {
        std::cout << "REDOING: " << e << std::endl;
        process_input(e);
    }
}

void Mapper::process_input(InputEvent e) {
    const State &cur_state = states.at(cur_layer_idx).at(cur_state_idx);
    const Transition &transition = cur_state.edges.contains(e)
                                       ? cur_state.edges[e]
                                       : cur_state.fallback_transition;
    apply_transition(transition, e);
}

void Mapper::check_queued_events() {
    if (queued_events.empty())
        return;
    uint64_t cur_time = current_time_ms();

    auto pred = [&](const std::pair<uint64_t, OutputEvent> &p) {
        return p.first > cur_time;
    };

    auto mid = std::partition(queued_events.begin(), queued_events.end(), pred);

    // Everything from mid..end are ready/expired
    if (mid != queued_events.end()) {
        QList<OutputEvent> ready_events;
        ready_events.reserve(static_cast<int>(queued_events.end() - mid));

        for (auto it = mid; it != queued_events.end(); ++it) {
            ready_events.append(std::move(it->second));
        }

        daemon.send_outputs(ready_events);

        // Just drop the tail in one shot
        queued_events.resize(std::distance(queued_events.begin(), mid));
    }
}
void Mapper::stop() { stopped = true; }

bool Mapper::waiting_for_timer() {
    std::lock_guard lock_guard(mtx);
    return current_timer.has_value() && queued_events.size() == 0;
}

size_t Mapper::processed_events() {
    std::lock_guard lock_guard(mtx);
    return processed_events_count;
}
