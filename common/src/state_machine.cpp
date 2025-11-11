#include "state_machine.h"
#include "event.h"
#include "profile.h"
#include "util.h"
#include <QVector>
#include <cassert>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

namespace {
struct StateMetadata {
    std::optional<Timer> timer;
    SequenceBehavior behavior;
};
std::pair<State, StateMetadata> generate_home_state() {
    return std::make_pair(
        State{QHash<InputEvent, Transition>{},
              {
                  .timer_ms = std::nullopt,
                  .outputs = QList<StateMachineOutputEvent>{BasicTranlation{}},
                  .new_state = HOME_STATE_IDX,
              },
              std::nullopt},
        StateMetadata{
            .timer = std::nullopt,
            .behavior = SequenceBehavior::Default,
        });
}

// Normalize timeouts so that subsequent Timers are converted into 1.
// These shouldn't be created in the app but figured it was better to handle
// then not. If the profile is invalid then nullopt is returned
std::optional<QList<SequenceTrigger>>
normailze_sequences(const QList<SequenceTrigger> &sequences) {
    QList<SequenceTrigger> normalized_sequences;
    for (const SequenceTrigger &sequence_trigger : sequences) {
        SequenceTrigger normalized_sequence;
        normalized_sequence.behavior = sequence_trigger.behavior;
        normalized_sequence.binds = sequence_trigger.binds;
        for (size_t i = 0; i < sequence_trigger.sequence.length(); i++) {
            auto resp = std::visit(
                overloaded{
                    [&](MinimumWait &min_wait)
                        -> std::optional<AdvancedTrigger> {
                        if (i == 0) {
                            return std::nullopt;
                        }
                        auto normalized_wait = min_wait;
                        size_t j = i + 1;
                        for (; j < sequence_trigger.sequence.length(); j++) {
                            if (std::holds_alternative<MinimumWait>(
                                    sequence_trigger.sequence[j])) {

                                normalized_wait.ms +=
                                    std::get<MinimumWait>(
                                        sequence_trigger.sequence[j])
                                        .ms;
                            } else if (std::holds_alternative<MaximumWait>(
                                           sequence_trigger.sequence[j])) {
                                return std::nullopt;
                            } else {
                                break;
                            }
                        }
                        i = j - 1;
                        return normalized_wait;
                    },
                    [&](MaximumWait &max_wait)
                        -> std::optional<AdvancedTrigger> {
                        if (i == 0) {
                            return std::nullopt;
                        }
                        auto normalized_wait = max_wait;
                        size_t j = i + 1;
                        for (; j < sequence_trigger.sequence.length(); j++) {
                            if (std::holds_alternative<MaximumWait>(
                                    sequence_trigger.sequence[j])) {

                                normalized_wait.ms +=
                                    std::get<MaximumWait>(
                                        sequence_trigger.sequence[j])
                                        .ms;
                            } else if (std::holds_alternative<MinimumWait>(
                                           sequence_trigger.sequence[j])) {
                            } else {
                                return std::nullopt;
                            }
                        }
                        i = j - 1;
                        return normalized_wait;
                    },
                    [&](auto &rest) -> std::optional<AdvancedTrigger> {
                        return rest;
                    },
                },
                sequence_trigger.sequence[i]);
            if (resp) {
                normalized_sequence.sequence.append(*resp);
            } else {
                return std::nullopt;
            }
        }
        normalized_sequences.append(normalized_sequence);
    }
    return normalized_sequences;
}
std::optional<InputEvent> trigger_to_input(AdvancedTrigger trigger) noexcept {
    return std::visit(
        overloaded{
            [&](const KeyPress &kp) -> std::optional<InputEvent> {
                return InputEvent::fromKey(kp.key_code, KeyEventType::Press);
            },
            [&](const KeyRelease &kr) -> std::optional<InputEvent> {
                return InputEvent::fromKey(kr.key_code, KeyEventType::Release);
            },
            [&](const MinimumWait &mw) -> std::optional<InputEvent> {
                return std::nullopt;
            },
            [&](const MaximumWait &mw) -> std::optional<InputEvent> {
                return std::nullopt;
            },
            [&](const AppTrigger &at) -> std::optional<InputEvent> {
                // Represent app triggers as an InputEvent carrying the app name.
                // This allows sequences that include app-focus/launch events to
                // be represented in the state machine instead of dropping
                // the trigger and causing .value() to throw.
                return InputEvent::fromApp(at.appName, KeyEventType::AppFocus);
            },
        },
        trigger);
}

} // namespace

// Generates a vector of State objects from a given Layer
std::optional<std::vector<State>> generate_states(const Layer &layer) {
    auto [home_state, home_state_metadata] = generate_home_state();
    std::vector<State> states{{home_state}};

    // Used to check that shared states are compatible. Not needed after the
    // machine is constructed
    std::vector<StateMetadata> state_metadata{{home_state_metadata}};

    auto normalized_sequences_opt =
        normailze_sequences(layer.sequence_remappings);
    if (!normalized_sequences_opt)
        return std::nullopt;

    auto normalized_sequences = *normalized_sequences_opt;
    // Construct the states for each sequence. These sequences
    // can share states if they are compatible. If an incompatablity is found
    // then std::nullopt is returned
    for (const SequenceTrigger &sequence_trigger : normalized_sequences) {
        size_t current_state_idx = HOME_STATE_IDX;
        SequenceBehavior cur_behavior = sequence_trigger.behavior;

        for (size_t i = 0; i < sequence_trigger.sequence.size(); ++i) {
            bool is_last = i + 1 == sequence_trigger.sequence.size();
            bool is_second_to_last = i + 2 == sequence_trigger.sequence.size();
            const AdvancedTrigger &trigger = sequence_trigger.sequence[i];

            QHash<InputEvent, Transition> edges;
            Transition fallback{
                .timer_ms = std::nullopt,
                .outputs = {},
                .new_state = HOME_STATE_IDX,
            };

            std::optional<Timer> timer = std::nullopt;
            std::optional<size_t> timer_ms = std::nullopt;

            InputEvent input_event = trigger_to_input(trigger).value();

            // Look ahead for a timer as it affects the current transition
            if (!is_last) {
                std::visit(overloaded{[&](const MinimumWait &t) {
                                          timer = t;
                                          timer_ms = t.ms;
                                          i++; // consume timer trigger
                                      },
                                      [&](const MaximumWait &t) {
                                          timer = t;
                                          timer_ms = t.ms;
                                          i++; // consume timer trigger
                                      },
                                      [&](auto &) {}},
                           sequence_trigger.sequence[i + 1]);
            }

            size_t next_state_idx;
            if (states.at(current_state_idx).edges.contains(input_event)) {
                // If this is the last bind in a sequence then we know that some
                // binds conflict
                if (is_last) {
                    // TODO: add logs
                    return std::nullopt;
                }
                Transition &existing_transition =
                    states.at(current_state_idx).edges[input_event];
                // Transition for this event alreay exits. Using the state
                // metadata ensure these states can be shared
                const StateMetadata &matadata =
                    state_metadata.at(existing_transition.new_state);
                // Differnt behavior means these are incompatable we could
                // somehow make them compatible in the future but seems
                // low priority
                if (matadata.behavior != cur_behavior) {
                    // TODO: add logs for error
                    return std::nullopt;
                }

                if (matadata.timer.has_value() && timer.has_value() &&
                    matadata.timer != timer) {
                    // TODO: add logs for error
                    return std::nullopt;
                }

                next_state_idx = existing_transition.new_state;
            } else {
                // If no Transition already exits for this input then we create
                // a new state and transition to said state
                Transition transition{.timer_ms = timer_ms};
                if (cur_behavior == SequenceBehavior::Release) {
                    transition.outputs = {BasicTranlation{input_event}};
                }

                // if this is the last trigger then we also output the given
                // bind and go home
                if (is_last) {
                    for (const Bind &x : sequence_trigger.binds) {
                        transition.outputs.append(x);
                    }
                    transition.new_state = HOME_STATE_IDX;
                }
                // If this is not the last trigger in the sequence then create a
                // new state
                else {
                    State new_state;
                    StateMetadata new_state_metadata{
                        .timer = timer,
                        .behavior = cur_behavior,
                    };
                    new_state.fallback_transition.new_state = HOME_STATE_IDX;
                    if (cur_behavior == SequenceBehavior::Default) {
                        for (size_t j = 0; j <= i; j++) {
                            auto input =
                                trigger_to_input(sequence_trigger.sequence[j]);
                            if (input) {
                                StateMachineOutputEvent event;
                                if (j == 0) {
                                    event = BasicTranlation{input};
                                } else {
                                    event = ProccessInput{*input};
                                }
                                new_state.fallback_transition.outputs.append(
                                    event);
                            }
                        }
                    }
                    new_state.fallback_transition.outputs.append(
                        BasicTranlation{});
                    transition.new_state = states.size();
                    next_state_idx = transition.new_state;
                    states.push_back(new_state);
                    state_metadata.push_back(new_state_metadata);
                }
                states.at(current_state_idx).edges[input_event] = transition;
            }

            State &next_state = states.at(next_state_idx);
            size_t final_state_idx = next_state_idx;
            if (timer) {
                std::visit(
                    overloaded{
                        [&](const MaximumWait &max_wait) {
                            if (!next_state.timer_transition) {
                                next_state.timer_transition =
                                    next_state.fallback_transition;
                            }
                        },
                        [&](const MinimumWait &min_wait) {
                            if (next_state.timer_transition) {
                                final_state_idx =
                                    next_state.timer_transition.value()
                                        .new_state;
                            } else {
                                if (is_second_to_last) {
                                    next_state.timer_transition =
                                        Transition{.new_state = HOME_STATE_IDX};

                                    for (const Bind &x :
                                         sequence_trigger.binds) {
                                        next_state.timer_transition.value()
                                            .outputs.append(x);
                                    }

                                } else {
                                    final_state_idx = states.size();
                                    State new_state;
                                    StateMetadata new_state_metadata{
                                        .timer = std::nullopt,
                                        .behavior = cur_behavior,
                                    };
                                    next_state.timer_transition = Transition{
                                        .new_state = final_state_idx};
                                    next_state.fallback_transition =
                                        next_state.fallback_transition;
                                    states.push_back(new_state);
                                    state_metadata.push_back(
                                        new_state_metadata);
                                }
                            }
                        },
                    },
                    *timer);
            }

            current_state_idx = final_state_idx;
        }
    }

    return states;
}
