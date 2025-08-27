#include "key_map.h"
#include "key_code.h"
#include <windows.h>

BiMap<UINT, KeyCode> int_to_keycode = {
    // Letters
    {'A', KeyCode::A},
    {'B', KeyCode::B},
    {'C', KeyCode::C},
    {'D', KeyCode::D},
    {'E', KeyCode::E},
    {'F', KeyCode::F},
    {'G', KeyCode::G},
    {'H', KeyCode::H},
    {'I', KeyCode::I},
    {'J', KeyCode::J},
    {'K', KeyCode::K},
    {'L', KeyCode::L},
    {'M', KeyCode::M},
    {'N', KeyCode::N},
    {'O', KeyCode::O},
    {'P', KeyCode::P},
    {'Q', KeyCode::Q},
    {'R', KeyCode::R},
    {'S', KeyCode::S},
    {'T', KeyCode::T},
    {'U', KeyCode::U},
    {'V', KeyCode::V},
    {'W', KeyCode::W},
    {'X', KeyCode::X},
    {'Y', KeyCode::Y},
    {'Z', KeyCode::Z},

    // Digits
    {'0', KeyCode::K0},
    {'1', KeyCode::K1},
    {'2', KeyCode::K2},
    {'3', KeyCode::K3},
    {'4', KeyCode::K4},
    {'5', KeyCode::K5},
    {'6', KeyCode::K6},
    {'7', KeyCode::K7},
    {'8', KeyCode::K8},
    {'9', KeyCode::K9},

    // Special Characters
    {VK_SPACE, KeyCode::Space},
    // {VK_RETURN, "Enter"},
    // {VK_ESCAPE, "Esc"},
    // {VK_ESCAPE, "Escape"},
    // {VK_TAB, "Tab"},
    // {VK_SHIFT, "Shift"},
    // {VK_CONTROL, "Ctrl"},
    // {VK_MENU, "Alt"},
    // {VK_BACK, "Backspace"},
    // {VK_PAUSE, "Pause"},
    // {VK_CAPITAL, "CapsLock"},
    //
    // // Function Keys
    // {VK_F1, "F1"},
    // {VK_F2, "F2"},
    // {VK_F3, "F3"},
    // {VK_F4, "F4"},
    // {VK_F5, "F5"},
    // {VK_F6, "F6"},
    // {VK_F7, "F7"},
    // {VK_F8, "F8"},
    // {VK_F9, "F9"},
    // {VK_F10, "F10"},
    // {VK_F11, "F11"},
    // {VK_F12, "F12"},
    //
    // // Special Symbols
    // {VK_OEM_3, "~"},
    // {VK_OEM_3, "`"},
    // {VK_OEM_MINUS, "-"},
    // {VK_OEM_PLUS, "="},
    // {VK_OEM_4, "["},
    // {VK_OEM_6, "]"},
    // {VK_OEM_5, "\\"},
    // {VK_OEM_1, ";"},
    // {VK_OEM_7, "'"},
    // {VK_OEM_COMMA, ","},
    // {VK_OEM_PERIOD, "."},
    // {VK_OEM_2, "/"},
    //
    // // Arrow Keys
    // {VK_UP, "Up"},
    // {VK_DOWN, "Down"},
    // {VK_LEFT, "Left"},
    // {VK_RIGHT, "Right"},
    //
    // // Other special keys
    // {VK_INSERT, "Insert"},
    // {VK_DELETE, "Delete"},
    // {VK_HOME, "Home"},
    // {VK_END, "End"},
    // {VK_PRIOR, "PageUp"},
    // {VK_NEXT, "PageDown"},
    // {VK_LWIN, "Cmd"},
    // {VK_LMENU},
    // "Alt"};
};
