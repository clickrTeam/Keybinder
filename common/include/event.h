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

struct AppFocusedEvent {
    QString app_name;
};

static QString normalize(const QString &s) {
    QString out;
    out.reserve(s.size());
    for (QChar ch : s) {
        if (!ch.isSpace()) out.append(ch.toLower());
    }
    return out;
}

inline bool operator==(const AppFocusedEvent &a, const AppFocusedEvent &b) {
    QString na = normalize(a.app_name);
    QString nb = normalize(b.app_name);
    if (na.isEmpty() || nb.isEmpty()) return false; // adjust if empty should count
    return na.contains(nb) || nb.contains(na);
}

inline uint qHash(const AppFocusedEvent &e, uint seed = 0) {
    return ::qHash("APPS", seed);
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
using OutputEvent = std::variant<KeyEvent, RunScript, AppLaunch>;

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
