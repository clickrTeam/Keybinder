#pragma once
#include "event.h"
#include "key_code.h"
#include <QDebug>
#include <QHash>
#include <profile.h>
#include <variant>

enum class KeyEventType {
    Press,
    Release,
};

struct InputEvent {
    KeyCode keycode;
    KeyEventType type;
};

inline bool operator==(const InputEvent &a, const InputEvent &b) {
    return a.keycode == b.keycode && a.type == b.type;
}

inline uint qHash(const InputEvent &key, uint seed = 0) {
    return ::qHash(static_cast<int>(key.keycode), seed) ^
           ::qHash(static_cast<int>(key.type), seed << 1);
}

// TODO: eventually these will become differnt however for now they can be the
// same as we just handle keys
using OutputEvent = std::variant<InputEvent, RunScript>;
