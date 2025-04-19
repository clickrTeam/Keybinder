#include "mapper.h"
#include "daemon.h"
#include "event.h"
#include <QTimer>
#include <mutex>
#include <optional>
#include <profile.h>

Mapper::Mapper(Profile profile) { set_profile(profile); }

Mapper::~Mapper() {}

void Mapper::set_daemon(Daemon *d) {
    std::lock_guard lock_guard(mtx);
    daemon = d;
}

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
             << " Lenght = " << profile.layers.size();
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

bool Mapper::map_input(InputEvent e) {
    std::lock_guard lock_guard(mtx);

    if (current_tap_sequence == std::nullopt && e.type == KeyEventType::Press &&
        tap_sequence_starts.contains(e.keycode)) {
        const auto &[new_tap_sequence, bind] = tap_sequence_starts[e.keycode];
        current_tap_sequence = std::make_tuple(
            std::ref(new_tap_sequence), std::ref(bind), 0, KeyEventType::Press);
    }

    // Handle tap sequences
    if (current_tap_sequence.has_value()) {
        auto &[tap_sequence, bind, current_key, current_state] =
            *current_tap_sequence;

        KeyCode expected_key = tap_sequence.key_sequence[current_key];
        // Got the next key
        if (expected_key == e.keycode && e.type == current_state) {

            if (current_state == KeyEventType::Press) {
                current_state = KeyEventType::Relase;
            } else {
                current_state = KeyEventType::Press;
                current_key++;
            }

            // TOOD Start a timer to expire. For now I can't as qtimers rely on
            // an event loop running on the current thread which is not
            // happening
            current_tap_sequence =
                std::make_tuple(tap_sequence, bind, current_key, current_state);

            if (current_key == tap_sequence.key_sequence.size()) {
                perform_bind(bind);
                current_tap_sequence = std::nullopt;
            }

            if (tap_sequence.behavior == TimedTriggerBehavior::Capture) {
                return true;
            }
        } else if (tap_sequence.behavior == TimedTriggerBehavior::Default) {
            // Send out all keys now that the trigger has failed if we have
            // capture release behavior
            QList<Bind> binds;
            for (size_t i = 0; i < current_key; i++) {
                auto key_code = tap_sequence.key_sequence[i];
                binds.push_back(PressKey{key_code});
                binds.push_back(ReleaseKey{key_code});
            }
        }
    }

    // Handle simple press and releases
    if (e.type == KeyEventType::Press &&
        key_press_triggers.contains(e.keycode)) {
        perform_bind(key_press_triggers[e.keycode]);
    } else if (e.type == KeyEventType::Relase &&
               key_release_triggers.contains(e.keycode)) {
        perform_bind(key_release_triggers[e.keycode]);
    } else {
        return false;
    }

    return true;
}

void Mapper::perform_bind(Bind &bind) {

    // using Bind = std::variant<PressKey, ReleaseKey, TapKey, SwapLayer>;
    std::visit(
        [&](auto &&bind) {
            using T = std::decay_t<decltype(bind)>;
            if constexpr (std::is_same_v<T, PressKey>) {
                daemon->send_keys(
                    {InputEvent{bind.key_code, KeyEventType::Press}});
            } else if constexpr (std::is_same_v<T, ReleaseKey>) {
                daemon->send_keys(
                    {InputEvent{bind.key_code, KeyEventType::Relase}});
            } else if constexpr (std::is_same_v<T, TapKey>) {
                daemon->send_keys(
                    {InputEvent{bind.key_code, KeyEventType::Relase},
                     InputEvent{bind.key_code, KeyEventType::Press}});

            } else if constexpr (std::is_same_v<T, SwapLayer>) {
                set_layer_inner(bind.new_layer);
            }
        },
        bind);
}
