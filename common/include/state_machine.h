#include "event.h"
#include "profile.h"
#include <QDebug>
#include <QHash>
#include <functional>
#include <variant>

struct TimerExpiredEvent {
    size_t timer_id;
};

inline bool operator==(const TimerExpiredEvent &a, const TimerExpiredEvent &b) {
    return a.timer_id == b.timer_id;
}

inline uint qHash(const TimerExpiredEvent &key, uint seed = 0) {
    qDebug() << "qHash(TimerExpiredEvent) called"; // DEBUG
    return ::qHash(key.timer_id, seed);
}

using StateMachineInputEvent = std::variant<InputEvent, TimerExpiredEvent>;

inline bool operator==(const StateMachineInputEvent &a,
                       const StateMachineInputEvent &b) {
    return a.index() == b.index() &&
           std::visit([](auto &&lhs, auto &&rhs) { return lhs == rhs; }, a, b);
}

// Added qHash for std::variant with debug
inline uint qHash(const StateMachineInputEvent &key, uint seed = 0) {
    qDebug() << "qHash(StateMachineInputEvent) called"; // DEBUG
    return std::visit([seed](auto &&v) { return qHash(v, seed); }, key);
}

struct ProccessInputs {
    QVector<InputEvent> keys;
};

struct SendOutput {
    OutputEvent e;
};

struct StartTimer {
    size_t state_id;
    size_t timer_id;
};

using StateMachineOutputEvent = std::variant<SendOutput, StartTimer>;

class BasicInputMappings {
    QHash<InputEvent, QVector<OutputEvent>> basic_map;

  public:
    // Constructor to take a prebuilt map
    explicit BasicInputMappings(QHash<InputEvent, QVector<OutputEvent>> map)
        : basic_map(std::move(map)) {}
    std::optional<QVector<OutputEvent>> get(const InputEvent &input) const {
        auto it = basic_map.find(input);
        if (it != basic_map.end()) {
            return *it;
        } else {
            return std::nullopt;
        }
    }
};

// This is a function type which takes an StateMachineInputEvent and returns a
// List of actions to take and a new state to transition to.
using StateMachineEventHandler =
    std::function<std::tuple<QVector<StateMachineOutputEvent>, size_t>(
        const BasicInputMappings &)>;

using StateMachineEventDefaultHandler =
    std::function<std::tuple<QVector<StateMachineOutputEvent>, size_t>(
        StateMachineInputEvent, const BasicInputMappings &)>;

class State {
  public:
    State(QHash<StateMachineInputEvent, StateMachineEventHandler> handlers,
          StateMachineEventDefaultHandler default_h)
        : event_handlers(std::move(handlers)),
          default_handler(std::move(default_h)) {}
    std::tuple<QVector<StateMachineOutputEvent>, size_t>
    handle_event(StateMachineInputEvent input,
                 const BasicInputMappings &basic_mappings) const {
        auto it = event_handlers.find(input);
        if (it != event_handlers.end()) {
            return it.value()(basic_mappings);
        } else {

            return default_handler(input, basic_mappings);
        }
    }

  private:
    QHash<StateMachineInputEvent, StateMachineEventHandler> event_handlers;
    StateMachineEventDefaultHandler default_handler;
};

QVector<State> generate_states(Layer);
