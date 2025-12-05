#include "key_map.h"
#include "key_code.h"
#include <windows.h>

// TODO ChatGPT generated please verify
BiMap<UINT, KeyCode> int_to_keycode = {
    // --- Letters ---
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

    // --- Number Row ---
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

    // --- Function Keys ---
    {VK_F1, KeyCode::F1},
    {VK_F2, KeyCode::F2},
    {VK_F3, KeyCode::F3},
    {VK_F4, KeyCode::F4},
    {VK_F5, KeyCode::F5},
    {VK_F6, KeyCode::F6},
    {VK_F7, KeyCode::F7},
    {VK_F8, KeyCode::F8},
    {VK_F9, KeyCode::F9},
    {VK_F10, KeyCode::F10},
    {VK_F11, KeyCode::F11},
    {VK_F12, KeyCode::F12},

    // --- Modifiers ---
    {VK_LSHIFT, KeyCode::LeftShift},
    {VK_RSHIFT, KeyCode::RightShift},
    {VK_LCONTROL, KeyCode::LeftControl},
    {VK_RCONTROL, KeyCode::RightControl},
    {VK_LMENU, KeyCode::LeftAlt},
    {VK_RMENU, KeyCode::RightAlt},
    {VK_LWIN, KeyCode::LeftSuper},
    {VK_RWIN, KeyCode::RightSuper},

    // --- Navigation / Control ---
    {VK_ESCAPE, KeyCode::Escape},
    {VK_TAB, KeyCode::Tab},
    {VK_RETURN, KeyCode::Enter},
    {VK_BACK, KeyCode::Backspace},
    {VK_SPACE, KeyCode::Space},
    {VK_INSERT, KeyCode::Insert},
    {VK_DELETE, KeyCode::Delete},
    {VK_HOME, KeyCode::Home},
    {VK_END, KeyCode::End},
    {VK_PRIOR, KeyCode::PageUp},
    {VK_NEXT, KeyCode::PageDown},
    {VK_SNAPSHOT, KeyCode::PrintScreen},
    {VK_PAUSE, KeyCode::Pause},
    {VK_APPS, KeyCode::Menu},
    {VK_CAPITAL, KeyCode::CapsLock},
    {VK_NUMLOCK, KeyCode::NumLock},
    {VK_SCROLL, KeyCode::ScrollLock},

    // --- Arrow Keys ---
    {VK_UP, KeyCode::ArrowUp},
    {VK_DOWN, KeyCode::ArrowDown},
    {VK_LEFT, KeyCode::ArrowLeft},
    {VK_RIGHT, KeyCode::ArrowRight},

    // --- Symbols / Punctuation ---
    {VK_OEM_MINUS, KeyCode::Minus},
    {VK_OEM_PLUS, KeyCode::Equals},
    {VK_OEM_4, KeyCode::LeftBracket},
    {VK_OEM_6, KeyCode::RightBracket},
    {VK_OEM_5, KeyCode::BackSlash},
    {VK_OEM_1, KeyCode::Semicolon},
    {VK_OEM_7, KeyCode::Apostrophe},
    {VK_OEM_3, KeyCode::Grave},
    {VK_OEM_COMMA, KeyCode::Comma},
    {VK_OEM_PERIOD, KeyCode::Period},
    {VK_OEM_2, KeyCode::ForwardSlash},

    // --- Numpad ---
    {VK_NUMPAD0, KeyCode::Numpad0},
    {VK_NUMPAD1, KeyCode::Numpad1},
    {VK_NUMPAD2, KeyCode::Numpad2},
    {VK_NUMPAD3, KeyCode::Numpad3},
    {VK_NUMPAD4, KeyCode::Numpad4},
    {VK_NUMPAD5, KeyCode::Numpad5},
    {VK_NUMPAD6, KeyCode::Numpad6},
    {VK_NUMPAD7, KeyCode::Numpad7},
    {VK_NUMPAD8, KeyCode::Numpad8},
    {VK_NUMPAD9, KeyCode::Numpad9},
    {VK_ADD, KeyCode::NumpadAdd},
    {VK_SUBTRACT, KeyCode::NumpadSubtract},
    {VK_MULTIPLY, KeyCode::NumpadMultiply},
    {VK_DIVIDE, KeyCode::NumpadDivide},
    {VK_DECIMAL, KeyCode::NumpadDecimal},
    {VK_RETURN, KeyCode::NumpadEnter}, // Shared Enter mapping for Numpad

    // --- Media / System ---
    {VK_VOLUME_UP, KeyCode::VolumeUp},
    {VK_VOLUME_DOWN, KeyCode::VolumeDown},
    {VK_VOLUME_MUTE, KeyCode::Mute},

    {VK_MEDIA_NEXT_TRACK, KeyCode::MediaNext},
    {VK_MEDIA_PREV_TRACK, KeyCode::MediaPrev},
    {VK_MEDIA_PLAY_PAUSE, KeyCode::MediaPlayPause},
    {VK_MEDIA_STOP, KeyCode::MediaStop},

    {VK_BROWSER_BACK, KeyCode::BrowserBack},
    {VK_BROWSER_FORWARD, KeyCode::BrowserForward},
    {VK_BROWSER_REFRESH, KeyCode::BrowserRefresh},
    {VK_BROWSER_STOP, KeyCode::BrowserStop},
    {VK_BROWSER_SEARCH, KeyCode::BrowserSearch},
    {VK_BROWSER_FAVORITES, KeyCode::BrowserFavorites},
    {VK_BROWSER_HOME, KeyCode::BrowserHome},

    {VK_LAUNCH_MAIL, KeyCode::Mail},
    {VK_LAUNCH_APP2, KeyCode::Calculator}, // Typically calculator
    {VK_LAUNCH_APP1, KeyCode::Computer},

    {VK_SLEEP, KeyCode::Sleep},
    {VK_POWER, KeyCode::Power}};
