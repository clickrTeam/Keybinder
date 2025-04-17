#ifndef READPROFILE_H
#define READPROFILE_H
#include "profile.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

// Define constants for each JSON key
#define PROFILE_NAME "profile_name"
#define PROFILE_LAYERS "layers"

#define LAYER_NAME "layer_name"
#define LAYER_KEYBINDS "remappings"

// BindType defines
#define BIND "Bind"
#define BINDTYPE_LINK "Link_Bind"
#define BINDTYPE_COMBO "Combo_Bind"
#define BINDTYPE_MACRO "Macro_Bind"
#define BINDTYPE_TIMEDMACRO "TimedMacro_Bind"
#define BINDTYPE_REPEAT "Repeat_Bind"
#define BINDTYPE_SWAPLAYER "SwapLayer_Bind"
#define BINDTYPE_APPOPEN "AppOpen_Bind"

// TriggerType defines
#define TRIGGER "Trigger"
#define TRIGGERTYPE_LINK "Link_Trigger"
#define TRIGGERTYPE_TIMED "Timed_Trigger"
#define TRIGGERTYPE_HOLD "Hold_Trigger"
#define TRIGGERTYPE_APPFOCUSED "App_Focus_Trigger"

// Inner values
#define KEY_TIME_PAIRS "key_time_pairs"
#define VALUE "value"
#define DELAY "delay"
#define WAIT "wait"
#define APP_NAME "app_name"
#define LAYER_NUM "layer_num"
#define TIME_DELAY "time_delay"
#define TIMES_TO_EXECUTE "times_to_execute"
#define CANCEL_TRIGGER "cancel_trigger"
#define CAPTURE "capture"
#define RELEASE "release"

// functions
Profile proccessProfile(const QString &profileFilePath);

Profile readProfile(QJsonDocument jsonDoc);
Layer readLayer(QJsonObject layer);
int stringToKey(QString keyString);
Trigger parseRemapping(const QJsonObject& remapping);

// String to key
#ifdef _WIN32
#include "windows.h"
const QMap<QString, WORD> keyMap = {
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
// const QMap<WORD, QString> vkToStringMap = []() {
//     QMap<WORD, QString> map;
//     for (auto it = keyMap.constBegin(); it != keyMap.constEnd(); ++it) {
//         map.insert(it.value(), it.key());  // Reverse key-value pairs
//     }
//     return map;
// }();
#elif defined(__APPLE__)
const QMap<QString, int> keyMap = {};
#elif defined(__linux__)
const QMap<QString, WORD> keyMap = {};
#else
#error "Unknown operating system"
#endif

#endif // READPROFILE_H
