#pragma once
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

// I didn't use these but I prbably should. Can update later
//  // Define constants for each JSON key
//  #define PROFILE_NAME "profile_name"
//  #define PROFILE_LAYERS "layers"
//
//  #define LAYER_NAME "layer_name"
//  #define LAYER_KEYBINDS "remappings"
//
//  // BindType defines
//  #define BIND "Bind"
//  #define BINDTYPE_LINK "Link_Bind"
//  #define BINDTYPE_COMBO "Combo_Bind"
//  #define BINDTYPE_MACRO "Macro_Bind"
//  #define BINDTYPE_TIMEDMACRO "TimedMacro_Bind"
//  #define BINDTYPE_REPEAT "Repeat_Bind"
//  #define BINDTYPE_SWAPLAYER "SwapLayer_Bind"
//  #define BINDTYPE_APPOPEN "AppOpen_Bind"
//
//  // TriggerType defines
//  #define TRIGGER "Trigger"
//  #define TRIGGERTYPE_LINK "Link_Trigger"
//  #define TRIGGERTYPE_TIMED "Timed_Trigger"
//  #define TRIGGERTYPE_HOLD "Hold_Trigger"
//  #define TRIGGERTYPE_APPFOCUSED "App_Focus_Trigger"
//
//  // Inner values
//  #define KEY_TIME_PAIRS "key_time_pairs"
//  #define VALUE "value"
//  #define DELAY "delay"
//  #define WAIT "wait"
//  #define APP_NAME "app_name"
//  #define LAYER_NUM "layer_num"
//  #define TIME_DELAY "time_delay"
//  #define TIMES_TO_EXECUTE "times_to_execute"
//  #define CANCEL_TRIGGER "cancel_trigger"
//  #define CAPTURE "capture"
//  #define RELEASE "release"

// clang-format off
// String to key
#ifdef _WIN32
#include "windows.h"
const QMap<QString, WORD> string_to_key_code = {
    // Letters
    {"A", 'A'}, {"B", 'B'}, {"C", 'C'}, {"D", 'D'}, {"E", 'E'},
    {"F", 'F'}, {"G", 'G'}, {"H", 'H'}, {"I", 'I'}, {"J", 'J'},
    {"K", 'K'}, {"L", 'L'}, {"M", 'M'}, {"N", 'N'}, {"O", 'O'},
    {"P", 'P'}, {"Q", 'Q'}, {"R", 'R'}, {"S", 'S'}, {"T", 'T'},
    {"U", 'U'}, {"V", 'V'}, {"W", 'W'}, {"X", 'X'}, {"Y", 'Y'},
    {"Z", 'Z'},

    // Digits
    {"0", '0'}, {"1", '1'}, {"2", '2'}, {"3", '3'}, {"4", '4'},
    {"5", '5'}, {"6", '6'}, {"7", '7'}, {"8", '8'}, {"9", '9'},

    // Special Characters
    {"Space", VK_SPACE}, {"Enter", VK_RETURN}, {"Esc", VK_ESCAPE},
    {"Tab", VK_TAB}, {"Shift", VK_SHIFT}, {"Ctrl", VK_CONTROL},
    {"Alt", VK_MENU}, {"Backspace", VK_BACK}, {"Pause", VK_PAUSE},
    {"CapsLock", VK_CAPITAL}, {"Escape", VK_ESCAPE}, {"Space", VK_SPACE},

    // Function Keys
    {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
    {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
    {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},

    // Special Symbols
    {"~", VK_OEM_3}, {"-", VK_OEM_MINUS}, {"=", VK_OEM_PLUS},
    {"[", VK_OEM_4}, {"]", VK_OEM_6}, {"\\", VK_OEM_5},
    {";", VK_OEM_1}, {"'", VK_OEM_7}, {",", VK_OEM_COMMA},
    {".", VK_OEM_PERIOD}, {"/", VK_OEM_2}, {"`", VK_OEM_3},

    // Arrow Keys
    {"Up", VK_UP}, {"Down", VK_DOWN}, {"Left", VK_LEFT}, {"Right", VK_RIGHT},

    // Other special keys
    {"Insert", VK_INSERT}, {"Delete", VK_DELETE}, {"Home", VK_HOME},
    {"End", VK_END}, {"PageUp", VK_PRIOR}, {"PageDown", VK_NEXT},
    {"Alt", VK_LMENU}, {"Cmd", VK_LWIN}, // TODO: find fix for RWIN, RMENU, MENU
};
// reverse the map
// const QMap<WORD, QString> key_code_to_string = []() {
//     QMap<WORD, QString> map;
//     for (auto it = keyMap.constBegin(); it != keyMap.constEnd(); ++it) {
//         map.insert(it.value(), it.key());  // Reverse key-value pairs
//     }
//     return map;
// }();
#elif defined(__APPLE__)
// Chat gpt generated need to verify
const QMap<QString, int> string_to_key_code = {
    // Letters
    {"A", 0}, {"S", 1}, {"D", 2}, {"F", 3}, {"H", 4},
    {"G", 5}, {"Z", 6}, {"X", 7}, {"C", 8}, {"V", 9},
    {"B", 11}, {"Q", 12}, {"W", 13}, {"E", 14}, {"R", 15},
    {"Y", 16}, {"T", 17}, {"1", 18}, {"2", 19}, {"3", 20},
    {"4", 21}, {"6", 22}, {"5", 23}, {"Equal", 24}, {"9", 25},
    {"7", 26}, {"Minus", 27}, {"8", 28}, {"0", 29}, {"RightBracket", 30},
    {"O", 31}, {"U", 32}, {"LeftBracket", 33}, {"I", 34}, {"P", 35},
    {"L", 37}, {"J", 38}, {"Quote", 39}, {"K", 40}, {"Semicolon", 41},
    {"Backslash", 42}, {"Comma", 43}, {"Slash", 44}, {"N", 45}, {"M", 46},
    {"Period", 47}, {"Grave", 50},

    // Special keys
    {"Return", 36}, {"Tab", 48}, {"Space", 49}, {"Delete", 51}, {"Escape", 53},

    // Modifier keys
    {"Command", 55}, {"Shift", 56}, {"CapsLock", 57}, {"Option", 58}, {"Control", 59},
    {"RightShift", 60}, {"RightOption", 61}, {"RightControl", 62}, {"Function", 63},

    // Arrow keys
    {"F17", 64}, {"VolumeUp", 72}, {"VolumeDown", 73}, {"Mute", 74},
    {"F18", 79}, {"F19", 80}, {"F20", 90},
    {"F5", 96}, {"F6", 97}, {"F7", 98}, {"F3", 99}, {"F8", 100}, {"F9", 101},
    {"F11", 103}, {"F13", 105}, {"F16", 106}, {"F14", 107}, {"F10", 109},
    {"F12", 111}, {"F15", 113}, {"Help", 114}, {"Home", 115}, {"PageUp", 116},
    {"ForwardDelete", 117}, {"F4", 118}, {"End", 119}, {"F2", 120},
    {"PageDown", 121}, {"F1", 122}, {"LeftArrow", 123}, {"RightArrow", 124},
    {"DownArrow", 125}, {"UpArrow", 126}
};

// reverse the map
const QMap<int, QString> key_code_to_string = []() {
    QMap<int, QString> map;
    for (auto it = string_to_key_code.constBegin(); it != string_to_key_code.constEnd(); ++it) {
        map.insert(it.value(), it.key());  // Reverse key-value pairs
    }
    return map;
}();

#elif defined(__linux__)
const QMap<QString, WORD> keyMap = {};
#else
#error "Unknown operating system"
#endif

