#include "mapper.h"
#include "daemon.h"
#include "event.h"
#include "key_channel.h"
#include "key_code.h"
#include "layer_indicator.h"
#include <QTimer>
#include <mutex>
#include <optional>
#include <profile.h>
#include <QApplication>

Mapper::Mapper(Profile profile, Daemon &daemon, KeyReceiver key_receiver)
    : daemon(daemon), key_receiver(key_receiver) {
    set_profile(profile);
}

Mapper::~Mapper() {}

void Mapper::set_profile(Profile p) {
    std::lock_guard lock_guard(mtx);
    profile = p;
    qDebug() << "Profile layer is: " << profile.default_layer;
    set_layer_inner(profile.default_layer);
}

bool Mapper::set_layer(size_t new_layer) {
    std::lock_guard lock_guard(mtx);
    if (new_layer >= this->profile.layers.size()) {
        return false;
    }
    set_layer_inner(new_layer);
    return true;
}

// must be called with mtx aquired
void Mapper::set_layer_inner(size_t new_layer) {
    cur_layer = new_layer;

    qDebug() << "Cur Layer: " << cur_layer
             << " Length = " << profile.layers.size();

    QTimer::singleShot(0, qApp, [layerName = profile.layers[new_layer].layer_name]() {
        new LayerIndicator(layerName, 1000);
    });


    key_press_triggers.clear();
    key_release_triggers.clear();
    tap_sequence_starts.clear();
    current_tap_sequence = std::nullopt;
    for (const auto &pair : profile.layers[cur_layer].remappings) {
        Trigger trigger = pair.first;
        Bind bind = pair.second;
        // trigger and bind are now copies
        std::visit(
            [this, bind](auto &&trigger) {
                using T = std::decay_t<decltype(trigger)>;
                if constexpr (std::is_same_v<T, KeyPress>) {
                    key_press_triggers.insert(trigger.key_code, bind);
                } else if constexpr (std::is_same_v<T, KeyRelease>) {
                    key_release_triggers.insert(trigger.key_code, bind);
                } else if constexpr (std::is_same_v<T, TapSequence>) {
                    tap_sequence_starts[trigger.key_sequence.at(0)] =
                        std::make_pair(trigger, bind);
                }
            },
            trigger);
    }
}

// TODO: mostly temp code will move to state Machines soon
void Mapper::start() {
    while (true) {
        // TODO: implement timouts
        auto key_opt = this->key_receiver.wait_key(std::nullopt);
        if (!key_opt.has_value()) {
            continue;
        }
        InputEvent e = key_opt.value();
        std ::lock_guard lock_guard(mtx);

        if (current_tap_sequence == std::nullopt &&
            e.type == KeyEventType::Press &&
            tap_sequence_starts.contains(e.keycode)) {
            const auto &[new_tap_sequence, bind] =
                tap_sequence_starts[e.keycode];
            current_tap_sequence =
                std::make_tuple(std::ref(new_tap_sequence), std::ref(bind), 0,
                                KeyEventType::Press);
        }

        // Handle tap sequences
        if (current_tap_sequence.has_value()) {
            auto &[tap_sequence, bind, current_key, current_state] =
                *current_tap_sequence;

            KeyCode expected_key = tap_sequence.key_sequence[current_key];
            qDebug() << "Checking for tap sequence cur_key: " << current_key
                     << "current_state: looking for "
                     << str_to_keycode.find_backward(expected_key);

            // Key matches the next tap in the sequence
            if (expected_key == e.keycode && e.type == current_state) {

                if (current_state == KeyEventType::Press) {
                    current_state = KeyEventType::Release;
                } else {
                    current_state = KeyEventType::Press;
                    current_key++;
                }

                // TOOD Start a timer to expire. For now I can't as qtimers rely
                // on an event loop running on the current thread which is not
                // happening. Should fix once mapper has its own thread. Also
                // QTimers are likley not accurate enough so we should roll our
                // own
                current_tap_sequence = std::make_tuple(
                    tap_sequence, bind, current_key, current_state);

                if (current_key == tap_sequence.key_sequence.size()) {
                    perform_binds({bind});
                    current_tap_sequence = std::nullopt;
                    qDebug() << "Tap Sequence Over";
                }

                if (tap_sequence.behavior != TimedTriggerBehavior::Release) {
                    continue;
                }
            } else if (tap_sequence.behavior == TimedTriggerBehavior::Default) {
                // Send out all keys now that the trigger has failed if we have
                // capture release behavior
                qDebug() << "Tap Sequence stopped, sending out caputred keys";
                QList<Bind> binds;
                for (size_t i = 0; i < current_key; i++) {
                    auto key_code = tap_sequence.key_sequence[i];
                    binds.push_back(PressKey{key_code});
                    binds.push_back(ReleaseKey{key_code});
                }
                // If we where waiting for a release then send the keydown
                if (current_state == KeyEventType::Release) {
                    auto key_code = tap_sequence.key_sequence[current_key];
                    binds.push_back(PressKey{key_code});
                }

                perform_binds(binds);
                current_tap_sequence = std::nullopt;
            } else {
                current_tap_sequence = std::nullopt;
            }
        }

        // Handle simple press and releases
        if (e.type == KeyEventType::Press &&
            key_press_triggers.contains(e.keycode)) {
            qDebug() << "Mapping keydown of : "
                     << str_to_keycode.find_backward(e.keycode);
            perform_binds({key_press_triggers[e.keycode]});
        } else if (e.type == KeyEventType::Release &&
                   key_release_triggers.contains(e.keycode)) {
            qDebug() << "Mapping keyup of : "
                     << str_to_keycode.find_backward(e.keycode);
            perform_binds({key_release_triggers[e.keycode]});
        } else {
            qDebug() << "No mappings found for :"
                     << str_to_keycode.find_backward(e.keycode);
            if (e.type == KeyEventType::Press) {
                perform_binds({PressKey{e.keycode}});
            } else {
                perform_binds({ReleaseKey{e.keycode}});
            }
        }
    }
}

void Mapper::perform_binds(const QList<Bind> &binds) {

    QList<InputEvent> events;

    for (const Bind &bind : binds) {
        std::visit(
            [&](auto &&bind) {
                using T = std::decay_t<decltype(bind)>;
                if constexpr (std::is_same_v<T, PressKey>) {
                    events.push_back(
                        {InputEvent{bind.key_code, KeyEventType::Press}});
                } else if constexpr (std::is_same_v<T, ReleaseKey>) {
                    events.push_back(
                        {InputEvent{bind.key_code, KeyEventType::Release}});
                } else if constexpr (std::is_same_v<T, TapKey>) {
                    events.append(
                        {InputEvent{bind.key_code, KeyEventType::Press},
                         InputEvent{bind.key_code, KeyEventType::Release}});
                } else if constexpr (std::is_same_v<T, SwapLayer>) {
                    set_layer_inner(bind.new_layer);
                } else if constexpr (std::is_same_v<T, Macro>) {
                    perform_binds(bind.binds);
                }
            },
            bind);
    }

    daemon.send_keys(events);
}
