#pragma once

#include "key_code.h"
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QString>
#include <cstddef>
#include <utility>
#include <variant>

enum class TimedTriggerBehavior {
    Capture,
    Release,
    Default,
};

TimedTriggerBehavior parse_behavior(const QString &str);

struct KeyPress {
    KeyCode key_code;
    static KeyPress from_json(const QJsonObject &obj);
};

struct KeyRelease {
    KeyCode key_code;
    static KeyRelease from_json(const QJsonObject &obj);
};

struct TapSequence {
    QList<KeyCode> key_sequence;
    TimedTriggerBehavior behavior;
    static TapSequence from_json(const QJsonObject &obj);
};

using Trigger = std::variant<KeyPress, KeyRelease, TapSequence>;

struct PressKey {
    KeyCode key_code;
    static PressKey from_json(const QJsonObject &obj);
};

struct ReleaseKey {
    KeyCode key_code;
    static ReleaseKey from_json(const QJsonObject &obj);
};

struct TapKey {
    KeyCode key_code;
    static TapKey from_json(const QJsonObject &obj);
};

struct SwapLayer {
    size_t new_layer;
    static SwapLayer from_json(const QJsonObject &obj);
};

struct Macro;

using Bind = std::variant<PressKey, ReleaseKey, TapKey, SwapLayer, Macro>;
Bind parse_bind(const QJsonObject &obj);

struct Macro
{
    QList<Bind> binds;
    static Macro from_json(const QJsonObject &obj);
};

struct Layer {
    QString layer_name;
    QList<std::pair<Trigger, Bind>> remappings;
    static Layer from_json(const QJsonObject &obj);
};

struct Profile {
    QString name;        // The name of the profile (e.g., "Default Profile")
    QList<Layer> layers; // List of layers in the profile
    size_t default_layer;
    static Profile from_json(const QJsonObject &obj);
    static Profile from_bytes(const QByteArray &bytes);
    static Profile from_file(const QString &filename);
    static Profile loadLatest();
};
