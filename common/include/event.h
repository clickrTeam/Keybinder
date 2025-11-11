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

struct AppOpenEvent {
    QString app_name;
};

inline bool operator==(const AppOpenEvent &a, const AppOpenEvent &b) {
    return a.app_name == b.app_name;
}

inline uint qHash(const AppOpenEvent &e, uint seed = 0) {
    return ::qHash(e.app_name, seed);
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

using InputEvent = std::variant<AppOpenEvent, KeyEvent>;

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
