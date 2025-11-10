#pragma once
#include "key_code.h"
#include <QDebug>
#include <QHash>
#include <QString>
#include <ostream>
#include <profile.h>
#include <variant>

enum class KeyEventType {
    Press,
    Release,
    AppFocus,
};

struct InputEvent {
    KeyEventType type;

    // payload: either a KeyCode or an app name string
    std::variant<KeyCode, QString> payload;

    // helpers
    static InputEvent fromKey(KeyCode k, KeyEventType t = KeyEventType::Press) {
        return InputEvent{t, std::variant<KeyCode, QString>(k)};
    }
    static InputEvent fromApp(const QString &name, KeyEventType t = KeyEventType::AppFocus) {
        return InputEvent{t, std::variant<KeyCode, QString>(name)};
    }

    bool isKey() const { return std::holds_alternative<KeyCode>(payload); }
    bool isApp() const { return std::holds_alternative<QString>(payload); }

    KeyCode key() const { return std::get<KeyCode>(payload); }          // call only if isKey()
    QString appName() const { return std::get<QString>(payload); }     // call only if isApp()
};

inline bool operator==(const InputEvent &a, const InputEvent &b) {
    return a.payload == b.payload && a.type == b.type;
}

inline uint qHash(const InputEvent &key, uint seed = 0) {
    uint h = ::qHash(static_cast<int>(key.type), seed);
    h ^= ::qHash(static_cast<int>(key.payload.index()), seed << 1);

    if (std::holds_alternative<KeyCode>(key.payload)) {
        h ^= ::qHash(static_cast<int>(std::get<KeyCode>(key.payload)), seed << 2);
    } else {
        h ^= ::qHash(std::get<QString>(key.payload), seed << 2);
    }
    return h;
}


// Output events can be key events, scripts, or app launches
using OutputEvent = std::variant<InputEvent, RunScript, AppLaunch>;

inline const char *to_string(KeyEventType t) {
    switch (t) {
    case KeyEventType::Press:
        return "Press";
    case KeyEventType::Release:
        return "Release";
    case KeyEventType::AppFocus:
        return "AppFocus";
    }
    return "Unknown";
}

// Usefull for failing tests
inline std::ostream &operator<<(std::ostream &os, const InputEvent &e) {
    os << "InputEvent::fromKey( type=" << static_cast<int>(e.type) << ", payload=";

    if (std::holds_alternative<KeyCode>(e.payload)) {
        os << "KeyCode(" << static_cast<int>(std::get<KeyCode>(e.payload)) << ")";
    } else if (std::holds_alternative<QString>(e.payload)) {
        os << "AppName(\"" << std::get<QString>(e.payload).toStdString() << "\")";
    } else {
        os << "empty";
    }

    os << " }";
    return os;
}

// Pretty print OutputEvent (variant)
inline std::ostream &operator<<(std::ostream &os, const OutputEvent &ev) {
    std::visit([&](auto &&arg) { os << arg; }, ev);
    return os;
}
