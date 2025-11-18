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
};

struct AppFocusedEvent {
    QString app_name;
};

inline bool operator==(const AppFocusedEvent &a, const AppFocusedEvent &b) {
    const auto al = a.app_name.toLower();
    const auto bl = b.app_name.toLower();
    return al.contains(bl) || bl.contains(al);
}

inline uint qHash(const AppFocusedEvent &e, uint seed = 0) {
    return ::qHash("APP", seed); // Required to match apps that are not complete matches.
}

struct KeyEvent {
    KeyCode keycode;
    KeyEventType type;
};

inline bool operator==(const KeyEvent &a, const KeyEvent &b) {
    return a.keycode == b.keycode && a.type == b.type;
}

inline uint qHash(const KeyEvent &key, uint seed = 0) {
    return ::qHash(static_cast<int>(key.keycode), seed) ^
           ::qHash(static_cast<int>(key.type), seed << 1);
}

using InputEvent = std::variant<AppFocusedEvent, KeyEvent>;

inline uint qHash(const InputEvent &e, uint seed = 0) {
    return std::visit([&](auto &&arg) { return ::qHash(arg, seed); }, e);
}

// TODO: eventually these will become differnt however for now they can be the
// same as we just handle keys
using OutputEvent = std::variant<KeyEvent, RunScript>;

inline const char *to_string(KeyEventType t) {
    switch (t) {
    case KeyEventType::Press:
        return "Press";
    case KeyEventType::Release:
        return "Release";
    }
}

// Usefull for failing tests
inline std::ostream &operator<<(std::ostream &os, const KeyEvent &e) {
    os << "InputEvent{ keycode=" << static_cast<int>(e.keycode)
       << ", type=" << to_string(e.type) << " }";
    return os;
}

// Pretty print OutputEvent (variant)
inline std::ostream &operator<<(std::ostream &os, const OutputEvent &ev) {
    std::visit([&](auto &&arg) { os << arg; }, ev);
    return os;
}
