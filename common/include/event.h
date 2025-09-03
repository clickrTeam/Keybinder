#pragma once
#include "key_code.h"
enum class KeyEventType {
    Press,
    Release,
};

struct InputEvent {
    KeyCode keycode;
    KeyEventType type;
};
