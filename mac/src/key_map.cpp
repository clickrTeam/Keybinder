#include "key_map.h"
#include <cstdint>

BiMap<uint16_t, KeyCode> int_to_keycode = {
    {4, KeyCode::A},     {5, KeyCode::B},   {6, KeyCode::C},
    {7, KeyCode::D},     {8, KeyCode::E},   {9, KeyCode::F},
    {10, KeyCode::G},    {11, KeyCode::H},  {12, KeyCode::I},
    {13, KeyCode::J},    {14, KeyCode::K},  {15, KeyCode::L},
    {16, KeyCode::M},    {17, KeyCode::N},  {18, KeyCode::O},
    {19, KeyCode::P},    {20, KeyCode::Q},  {21, KeyCode::R},
    {22, KeyCode::S},    {23, KeyCode::T},  {24, KeyCode::U},
    {25, KeyCode::V},    {26, KeyCode::W},  {27, KeyCode::X},
    {28, KeyCode::Y},    {29, KeyCode::Z},

    {30, KeyCode::K1},   {31, KeyCode::K2}, {32, KeyCode::K3},
    {33, KeyCode::K4},   {34, KeyCode::K5}, {35, KeyCode::K6},
    {36, KeyCode::K7},   {37, KeyCode::K8}, {38, KeyCode::K9},
    {39, KeyCode::K0},

    {44, KeyCode::Space}

};
