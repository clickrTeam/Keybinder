#include "key_map.h"
#include <QHash>
#include <QString>

KeyMap::KeyMap() {
    QList<QPair<QString, int>> string_to_key = create_list();
    foreach (auto pair, string_to_key) {
        insert(pair.first, pair.second);
    }
}

void KeyMap::insert(const QString &name, int keyCode) {
    string_to_key_map[name] = keyCode;
    key_to_string_map[keyCode] = name;
}

void KeyMap::remove(const QString &name, int keyCode) {
    string_to_key_map.remove(name);
    key_to_string_map.remove(keyCode);
}

int KeyMap::string_to_key_code(const QString &name) const {
    return string_to_key_map.value(name, -1);
}

QString KeyMap::key_code_to_string(int keyCode) const {
    return key_to_string_map.value(keyCode, QString());
}

bool KeyMap::contains_string(const QString &name) const {
    return string_to_key_map.contains(name);
}

bool KeyMap::contains_key_code(int keyCode) const {
    return key_to_string_map.contains(keyCode);
}

#ifdef _WIN32
#include "windows.h"

QList<QPair<QString, int>> KeyMap::create_list() {
    // TODO: ChatGPT generated verify this
    return {// Letters
            {"A", 'A'},
            {"B", 'B'},
            {"C", 'C'},
            {"D", 'D'},
            {"E", 'E'},
            {"F", 'F'},
            {"G", 'G'},
            {"H", 'H'},
            {"I", 'I'},
            {"J", 'J'},
            {"K", 'K'},
            {"L", 'L'},
            {"M", 'M'},
            {"N", 'N'},
            {"O", 'O'},
            {"P", 'P'},
            {"Q", 'Q'},
            {"R", 'R'},
            {"S", 'S'},
            {"T", 'T'},
            {"U", 'U'},
            {"V", 'V'},
            {"W", 'W'},
            {"X", 'X'},
            {"Y", 'Y'},
            {"Z", 'Z'},

            // Digits
            {"0", '0'},
            {"1", '1'},
            {"2", '2'},
            {"3", '3'},
            {"4", '4'},
            {"5", '5'},
            {"6", '6'},
            {"7", '7'},
            {"8", '8'},
            {"9", '9'},

            // Special Characters
            {"Space", VK_SPACE},
            {"Enter", VK_RETURN},
            {"Esc", VK_ESCAPE},
            {"Escape", VK_ESCAPE},
            {"Tab", VK_TAB},
            {"Shift", VK_SHIFT},
            {"Ctrl", VK_CONTROL},
            {"Alt", VK_MENU},
            {"Backspace", VK_BACK},
            {"Pause", VK_PAUSE},
            {"CapsLock", VK_CAPITAL},

            // Function Keys
            {"F1", VK_F1},
            {"F2", VK_F2},
            {"F3", VK_F3},
            {"F4", VK_F4},
            {"F5", VK_F5},
            {"F6", VK_F6},
            {"F7", VK_F7},
            {"F8", VK_F8},
            {"F9", VK_F9},
            {"F10", VK_F10},
            {"F11", VK_F11},
            {"F12", VK_F12},

            // Special Symbols
            {"~", VK_OEM_3},
            {"`", VK_OEM_3},
            {"-", VK_OEM_MINUS},
            {"=", VK_OEM_PLUS},
            {"[", VK_OEM_4},
            {"]", VK_OEM_6},
            {"\\", VK_OEM_5},
            {";", VK_OEM_1},
            {"'", VK_OEM_7},
            {",", VK_OEM_COMMA},
            {".", VK_OEM_PERIOD},
            {"/", VK_OEM_2},

            // Arrow Keys
            {"Up", VK_UP},
            {"Down", VK_DOWN},
            {"Left", VK_LEFT},
            {"Right", VK_RIGHT},

            // Other special keys
            {"Insert", VK_INSERT},
            {"Delete", VK_DELETE},
            {"Home", VK_HOME},
            {"End", VK_END},
            {"PageUp", VK_PRIOR},
            {"PageDown", VK_NEXT},
            {"Cmd", VK_LWIN},
            {"Alt", VK_LMENU}};
}

// const QMap<QString, WORD> string_to_key_code = {
//     // Letters
//     {"A", 'A'}, {"B", 'B'}, {"C", 'C'}, {"D", 'D'}, {"E", 'E'},
//     {"F", 'F'}, {"G", 'G'}, {"H", 'H'}, {"I", 'I'}, {"J", 'J'},
//     {"K", 'K'}, {"L", 'L'}, {"M", 'M'}, {"N", 'N'}, {"O", 'O'},
//     {"P", 'P'}, {"Q", 'Q'}, {"R", 'R'}, {"S", 'S'}, {"T", 'T'},
//     {"U", 'U'}, {"V", 'V'}, {"W", 'W'}, {"X", 'X'}, {"Y", 'Y'},
//     {"Z", 'Z'},

//     // Digits
//     {"0", '0'}, {"1", '1'}, {"2", '2'}, {"3", '3'}, {"4", '4'},
//     {"5", '5'}, {"6", '6'}, {"7", '7'}, {"8", '8'}, {"9", '9'},

//     // Special Characters
//     {"Space", VK_SPACE}, {"Enter", VK_RETURN}, {"Esc", VK_ESCAPE},
//     {"Tab", VK_TAB}, {"Shift", VK_SHIFT}, {"Ctrl", VK_CONTROL},
//     {"Alt", VK_MENU}, {"Backspace", VK_BACK}, {"Pause", VK_PAUSE},
//     {"CapsLock", VK_CAPITAL}, {"Escape", VK_ESCAPE}, {"Space", VK_SPACE},

//     // Function Keys
//     {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
//     {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
//     {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},

//     // Special Symbols
//     {"~", VK_OEM_3}, {"-", VK_OEM_MINUS}, {"=", VK_OEM_PLUS},
//     {"[", VK_OEM_4}, {"]", VK_OEM_6}, {"\\", VK_OEM_5},
//     {";", VK_OEM_1}, {"'", VK_OEM_7}, {",", VK_OEM_COMMA},
//     {".", VK_OEM_PERIOD}, {"/", VK_OEM_2}, {"`", VK_OEM_3},

//     // Arrow Keys
//     {"Up", VK_UP}, {"Down", VK_DOWN}, {"Left", VK_LEFT}, {"Right", VK_RIGHT},

//     // Other special keys
//     {"Insert", VK_INSERT}, {"Delete", VK_DELETE}, {"Home", VK_HOME},
//     {"End", VK_END}, {"PageUp", VK_PRIOR}, {"PageDown", VK_NEXT},
//     {"Alt", VK_LMENU}, {"Cmd", VK_LWIN}, // TODO: find fix for RWIN, RMENU,
//     MENU
// };
// reverse the map
// const QMap<WORD, QString> key_code_to_string = []() {
//     QMap<WORD, QString> map;
//     for (auto it = keyMap.constBegin(); it != keyMap.constEnd(); ++it) {
//         map.insert(it.value(), it.key());  // Reverse key-value pairs
//     }
//     return map;
// }();
#elif defined(__APPLE__)

QList<QPair<QString, int>> KeyMap::create_list() {
    return {// Letters A–Z starting at 4
            {"A", 4},
            {"B", 5},
            {"C", 6},
            {"D", 7},
            {"E", 8},
            {"F", 9},
            {"G", 10},
            {"H", 11},
            {"I", 12},
            {"J", 13},
            {"K", 14},
            {"L", 15},
            {"M", 16},
            {"N", 17},
            {"O", 18},
            {"P", 19},
            {"Q", 20},
            {"R", 21},
            {"S", 22},
            {"T", 23},
            {"U", 24},
            {"V", 25},
            {"W", 26},
            {"X", 27},
            {"Y", 28},
            {"Z", 29},

            // Numbers 1–9 starting at 30
            {"1", 30},
            {"2", 31},
            {"3", 32},
            {"4", 33},
            {"5", 34},
            {"6", 35},
            {"7", 36},
            {"8", 37},
            {"9", 38},
            {"0", 39},

            {"Space", 44}

    };
}

// // Chat gpt generated need to verify
// const QMap<QString, int> string_to_key_code = {
//     // Letters A–Z starting at 4
//     {"A", 4},  {"B", 5},  {"C", 6},  {"D", 7},  {"E", 8},
//     {"F", 9},  {"G", 10}, {"H", 11}, {"I", 12}, {"J", 13},
//     {"K", 14}, {"L", 15}, {"M", 16}, {"N", 17}, {"O", 18},
//     {"P", 19}, {"Q", 20}, {"R", 21}, {"S", 22}, {"T", 23},
//     {"U", 24}, {"V", 25}, {"W", 26}, {"X", 27}, {"Y", 28},
//     {"Z", 29},

//     // Numbers 1–9 starting at 30
//     {"1", 30}, {"2", 31}, {"3", 32}, {"4", 33}, {"5", 34},
//     {"6", 35}, {"7", 36}, {"8", 37}, {"9", 38}, {"0" ,39}
//     // TODO:  symbols, function keys, arrows, modifiers, etc.
// };

// // reverse the map
// const QMap<int, QString> key_code_to_string = []() {
//     QMap<int, QString> map;
//     for (auto it = string_to_key_code.constBegin(); it !=
//     string_to_key_code.constEnd(); ++it) {
//         map.insert(it.value(), it.key());  // Reverse key-value pairs
//     }
//     return map;
// }();

#elif defined(__linux__)
#include <linux/input-event-codes.h>
QList<QPair<QString, int>> KeyMap::create_list() {
    // TODO: ChatGPT generated verify this
    return {// Letters
            {"A", KEY_A},
            {"B", KEY_B},
            {"C", KEY_C},
            {"D", KEY_D},
            {"E", KEY_E},
            {"F", KEY_F},
            {"G", KEY_G},
            {"H", KEY_H},
            {"I", KEY_I},
            {"J", KEY_J},
            {"K", KEY_K},
            {"L", KEY_L},
            {"M", KEY_M},
            {"N", KEY_N},
            {"O", KEY_O},
            {"P", KEY_P},
            {"Q", KEY_Q},
            {"R", KEY_R},
            {"S", KEY_S},
            {"T", KEY_T},
            {"U", KEY_U},
            {"V", KEY_V},
            {"W", KEY_W},
            {"X", KEY_X},
            {"Y", KEY_Y},
            {"Z", KEY_Z},

            // Digits
            {"0", KEY_0},
            {"1", KEY_1},
            {"2", KEY_2},
            {"3", KEY_3},
            {"4", KEY_4},
            {"5", KEY_5},
            {"6", KEY_6},
            {"7", KEY_7},
            {"8", KEY_8},
            {"9", KEY_9},

            // Special Characters
            {"Space", KEY_SPACE},
            {"Enter", KEY_ENTER},
            {"Esc", KEY_ESC},
            {"Escape", KEY_ESC},
            {"Tab", KEY_TAB},
            {"Shift", KEY_LEFTSHIFT},
            {"Ctrl", KEY_LEFTCTRL},
            {"Alt", KEY_LEFTALT},
            {"Backspace", KEY_BACKSPACE},
            {"Pause", KEY_PAUSE},
            {"CapsLock", KEY_CAPSLOCK},

            // Function Keys
            {"F1", KEY_F1},
            {"F2", KEY_F2},
            {"F3", KEY_F3},
            {"F4", KEY_F4},
            {"F5", KEY_F5},
            {"F6", KEY_F6},
            {"F7", KEY_F7},
            {"F8", KEY_F8},
            {"F9", KEY_F9},
            {"F10", KEY_F10},
            {"F11", KEY_F11},
            {"F12", KEY_F12},

            // Special Symbols
            {"~", KEY_GRAVE},
            {"`", KEY_GRAVE},
            {"-", KEY_MINUS},
            {"=", KEY_EQUAL},
            {"[", KEY_LEFTBRACE},
            {"]", KEY_RIGHTBRACE},
            {"\\", KEY_BACKSLASH},
            {";", KEY_SEMICOLON},
            {"'", KEY_APOSTROPHE},
            {",", KEY_COMMA},
            {".", KEY_DOT},
            {"/", KEY_SLASH},

            // Arrow Keys
            {"Up", KEY_UP},
            {"Down", KEY_DOWN},
            {"Left", KEY_LEFT},
            {"Right", KEY_RIGHT},

            // Other special keys
            {"Insert", KEY_INSERT},
            {"Delete", KEY_DELETE},
            {"Home", KEY_HOME},
            {"End", KEY_END},
            {"PageUp", KEY_PAGEUP},
            {"PageDown", KEY_PAGEDOWN},
            {"Cmd", KEY_LEFTMETA},
            {"Super", KEY_LEFTMETA},
            {"Meta", KEY_LEFTMETA},
            {"Menu", KEY_MENU}};
}
#else
#error "Unknown operating system"
#endif
