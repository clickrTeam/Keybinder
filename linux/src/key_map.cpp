#include "key_map.h"
#include "key_code.h"
#include <cstdint>
#include <linux/input-event-codes.h>

// TODO: ChatGPT generated verify this
BiMap<uint16_t, KeyCode> int_to_keycode = {
    {KEY_A, KeyCode::A},
    {KEY_B, KeyCode::B},
    {KEY_C, KeyCode::C},
    {KEY_D, KeyCode::D},
    {KEY_E, KeyCode::E},
    {KEY_F, KeyCode::F},
    {KEY_G, KeyCode::G},
    {KEY_H, KeyCode::H},
    {KEY_I, KeyCode::I},
    {KEY_J, KeyCode::J},
    {KEY_K, KeyCode::K},
    {KEY_L, KeyCode::L},
    {KEY_M, KeyCode::M},
    {KEY_N, KeyCode::N},
    {KEY_O, KeyCode::O},
    {KEY_P, KeyCode::P},
    {KEY_Q, KeyCode::Q},
    {KEY_R, KeyCode::R},
    {KEY_S, KeyCode::S},
    {KEY_T, KeyCode::T},
    {KEY_U, KeyCode::U},
    {KEY_V, KeyCode::V},
    {KEY_W, KeyCode::W},
    {KEY_X, KeyCode::X},
    {KEY_Y, KeyCode::Y},
    {KEY_Z, KeyCode::Z},

    // Digits
    {KEY_0, KeyCode::K0},
    {KEY_1, KeyCode::K1},
    {KEY_2, KeyCode::K2},
    {KEY_3, KeyCode::K3},
    {KEY_4, KeyCode::K4},
    {KEY_5, KeyCode::K5},
    {KEY_6, KeyCode::K6},
    {KEY_7, KeyCode::K7},
    {KEY_8, KeyCode::K8},
    {KEY_9, KeyCode::K9},

    // Special Characters
    {KEY_SPACE, KeyCode::Space},
    // TODO: add to keycode
    //  {"Enter", KEY_ENTER},
    //  {KEY_ESC, KeyCode::Esc},
    //  {KEY_ESC, KeyCode::Escape},
    //  {KEY_TAB, KeyCode::Tab},
    //  {KEY_LEFTSHIFT, KeyCode::Shift},
    //  {KEY_LEFTCTRL, KeyCode::Ctrl},
    //  {KEY_LEFTALT, KeyCode::Alt},
    //  {KEY_BACKSPACE, KeyCode::Backspace},
    //  {KEY_PAUSE, KeyCode::Pause},
    //  {KEY_CAPSLOCK, KeyCode::CapsLock},
    //
    //  // Function Keys
    //  {KEY_F1, KeyCode::F1},
    //  {KEY_F2, KeyCode::F2},
    //  {KEY_F3, KeyCode::F3},
    //  {KEY_F4, KeyCode::F4},
    //  {KEY_F5, KeyCode::F5},
    //  {KEY_F6, KeyCode::F6},
    //  {KEY_F7, KeyCode::F7},
    //  {KEY_F8, KeyCode::F8},
    //  {KEY_F9, KeyCode::F9},
    //  {KEY_F10, KeyCode::F10},
    //  {KEY_F11, KeyCode::F11},
    //  {KEY_F12, KeyCode::F12},

    // // Special Symbols
    // {"~", KEY_GRAVE},
    // {"`", KEY_GRAVE},
    // {"-", KEY_MINUS},
    // {"=", KEY_EQUAL},
    // {"[", KEY_LEFTBRACE},
    // {"]", KEY_RIGHTBRACE},
    // {"\\", KEY_BACKSLASH},
    // {";", KEY_SEMICOLON},
    // {"'", KEY_APOSTROPHE},
    // {",", KEY_COMMA},
    // {".", KEY_DOT},
    // {"/", KEY_SLASH},
    //
    // // Arrow Keys
    // {"Up", KEY_UP},
    // {"Down", KEY_DOWN},
    // {"Left", KEY_LEFT},
    // {"Right", KEY_RIGHT},
    //
    // // Other special keys
    // {"Insert", KEY_INSERT},
    // {"Delete", KEY_DELETE},
    // {"Home", KEY_HOME},
    // {"End", KEY_END},
    // {"PageUp", KEY_PAGEUP},
    // {"PageDown", KEY_PAGEDOWN},
    // {"Cmd", KEY_LEFTMETA},
    // {"Super", KEY_LEFTMETA},
    // {"Meta", KEY_LEFTMETA},
    // {"Menu", KEY_MENU}};
};
