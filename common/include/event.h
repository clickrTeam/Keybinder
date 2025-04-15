#pragma once
enum class KeyEventType {
    Press,
    Release,
};

struct InputEvent {
    int keycode;
    KeyEventType type;
};
