#pragma once
#include "bi_map.h"
#include <QHash>
#include <QtGlobal> // Add this line
#include <cstdint>
// TODO: implement more keys
enum class KeyCode : std::uint16_t {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    K0,
    K1,
    K2,
    K3,
    K4,
    K5,
    K6,
    K7,
    K8,
    K9,
    Space,
};

// used to hash the KeyCode type
inline uint qHash(KeyCode key, uint seed = 0) noexcept {
    return qHash(static_cast<std::uint16_t>(key), seed);
}

extern BiMap<QString, KeyCode> str_to_keycode;
