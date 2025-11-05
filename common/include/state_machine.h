#include "event.h"
#include "profile.h"
#include <QDebug>
#include <QHash>
#include <optional>
#include <variant>

constexpr size_t HOME_STATE_IDX = 0;

// tell the maper to remap some inputs. If it is none that means to remap the
// latest mapped input
struct ProccessInput {
    std::optional<InputEvent> event;
};

inline bool operator==(const ProccessInput &a, const ProccessInput &b) {
    return a.event == b.event;
}

inline uint qHash(const ProccessInput &i, uint seed = 0) {
    return i.event ? qHash(*i.event, seed) : qHash(0, seed);
}

struct BasicTranlation {
    std::optional<InputEvent> event;
};

inline bool operator==(const BasicTranlation &a, const BasicTranlation &b) {
    return a.event == b.event;
}

inline uint qHash(const BasicTranlation &t, uint seed = 0) {
    return t.event ? qHash(*t.event, seed) : qHash(0, seed);
}

using StateMachineOutputEvent =
    std::variant<ProccessInput, BasicTranlation, Bind>;

inline uint qHash(const StateMachineOutputEvent &e, uint seed = 0) {
    return std::visit(
        [&](auto const &value) {
            seed ^= qHash(e.index(), seed);
            return qHash(value, seed);
        },
        e);
}

struct Transition {
    std::optional<size_t> timer_ms;
    QList<StateMachineOutputEvent> outputs;
    size_t new_state;
};

inline bool operator==(const Transition &a, const Transition &b) {
    return a.timer_ms == b.timer_ms && a.outputs == b.outputs &&
           a.new_state == b.new_state;
}

inline uint qHash(const Transition &t, uint seed = 0) {
    if (t.timer_ms) {
        seed ^= qHash(*t.timer_ms, seed);
    } else {
        seed ^= qHash(0, seed);
    }
    for (const auto &out : t.outputs) {
        seed ^= qHash(out, seed);
    }
    seed ^= qHash(t.new_state, seed);
    return seed;
}

struct State {
    QHash<InputEvent, Transition> edges;
    Transition fallback_transition;
    std::optional<Transition> timer_transition;
};

std::optional<std::vector<State>> generate_states(const Layer &);
