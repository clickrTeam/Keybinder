#include "key_code.h"
#include "bi_map.h"
#include <QString>

BiMap<QString, KeyCode> str_to_keycode = {
    {"A", KeyCode::A},        {"B", KeyCode::B},  {"C", KeyCode::C},
    {"D", KeyCode::D},        {"E", KeyCode::E},  {"F", KeyCode::F},
    {"G", KeyCode::G},        {"H", KeyCode::H},  {"I", KeyCode::I},
    {"J", KeyCode::J},        {"K", KeyCode::K},  {"L", KeyCode::L},
    {"M", KeyCode::M},        {"N", KeyCode::N},  {"O", KeyCode::O},
    {"P", KeyCode::P},        {"Q", KeyCode::Q},  {"R", KeyCode::R},
    {"S", KeyCode::S},        {"T", KeyCode::T},  {"U", KeyCode::U},
    {"V", KeyCode::V},        {"W", KeyCode::W},  {"X", KeyCode::X},
    {"Y", KeyCode::Y},        {"Z", KeyCode::Z},  {"0", KeyCode::K0},
    {"1", KeyCode::K1},       {"2", KeyCode::K2}, {"3", KeyCode::K3},
    {"4", KeyCode::K4},       {"5", KeyCode::K5}, {"6", KeyCode::K6},
    {"7", KeyCode::K7},       {"8", KeyCode::K8}, {"9", KeyCode::K9},
    {"Space", KeyCode::Space}

};
