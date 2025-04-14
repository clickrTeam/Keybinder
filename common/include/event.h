#pragma once
enum class KeyEventType {
    Press,
    Relase,
};

struct InputEvent {
    int keycode;
    KeyEventType type;
};
