#pragma once

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QString>
#include <cstddef>
#include <utility>
#include <variant>

// enum class KeyCode {
//     A = 0,
//     B = 1,
//     C = 2,
//     D = 3,
//     E = 4,
//     // TOOD
// };

enum class TimedBindBehavior {
    Capture,
    Release,
    Default,
};

// Type alias would make it easier to change to an enum in the future if needed
using KeyCode = int;

struct KeyPress {
    KeyCode keycode;
    static KeyPress from_json(const QJsonObject &obj);
};

struct KeyRelease {
    KeyCode keycode;
    static KeyRelease from_json(const QJsonObject &obj);
};

struct TapSequence {
    QList<KeyCode> key_sequence;
    TimedBindBehavior behavior;
    static TapSequence from_json(const QJsonObject &obj);
    static TimedBindBehavior parseBehavior(const QString &str);
};

using Trigger = std::variant<KeyPress, KeyRelease, TapSequence>;

struct PressKey {
    KeyCode keycode;
    static PressKey from_json(const QJsonObject &obj);
};

struct ReleaseKey {
    KeyCode keycode;
    static ReleaseKey from_json(const QJsonObject &obj);
};

struct TapKey {
    KeyCode keycode;
    static TapKey from_json(const QJsonObject &obj);
};

struct SwapLayer {
    size_t new_layer;
    static SwapLayer from_json(const QJsonObject &obj);
};

using Bind = std::variant<PressKey, ReleaseKey, TapKey, SwapLayer>;

struct Layer {
  public:
    QString layerName;
    QList<std::pair<Trigger, Bind>> remappings;
    static Layer from_json(const QJsonObject &obj);
};

struct Profile {
  public:
    QString name;        // The name of the profile (e.g., "Default Profile")
    QList<Layer> layers; // List of layers in the profile
    static Profile from_json(const QJsonObject &obj);
};
