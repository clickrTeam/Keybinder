#pragma once

#include "key_code.h"
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QString>
#include <cstddef>
#include <utility>
#include <variant>

enum class SequenceBehavior {
    Capture,
    Release,
    Default,
};

SequenceBehavior parse_behavior(const QString &str);
KeyCode parse_key(const QString &str);

struct KeyPress {
    KeyCode key_code;
    static KeyPress from_json(const QJsonObject &obj);
    bool operator==(const KeyPress &other) const noexcept {
        return key_code == other.key_code;
    }
};

struct KeyRelease {
    KeyCode key_code;
    static KeyRelease from_json(const QJsonObject &obj);
    bool operator==(const KeyRelease &other) const noexcept {
        return key_code == other.key_code;
    }
};

struct MinimumWait {
    size_t ms;
    bool operator==(const MinimumWait &other) const noexcept {
        return ms == other.ms;
    }
    static MinimumWait from_json(const QJsonObject &obj);
};

struct MaximumWait {
    size_t ms;
    bool operator==(const MaximumWait &other) const noexcept {
        return ms == other.ms;
    }
    static MaximumWait from_json(const QJsonObject &obj);
};
using Timer = std::variant<MinimumWait, MaximumWait>;

using BasicTrigger = std::variant<KeyPress, KeyRelease>;
BasicTrigger parse_basic_trigger(const QJsonObject &obj);

using AdvancedTrigger =
    std::variant<KeyPress, KeyRelease, MinimumWait, MaximumWait>;

AdvancedTrigger parse_trigger(const QJsonObject &obj);

struct PressKey {
    KeyCode key_code;
    bool operator==(const PressKey &other) const noexcept {
        return key_code == other.key_code;
    }
    static PressKey from_json(const QJsonObject &obj);
};

struct ReleaseKey {
    KeyCode key_code;
    bool operator==(const ReleaseKey &other) const noexcept {
        return key_code == other.key_code;
    }
    static ReleaseKey from_json(const QJsonObject &obj);
};

struct SwapLayer {
    size_t new_layer;
    bool operator==(const SwapLayer &other) const noexcept {
        return new_layer == other.new_layer;
    }
    static SwapLayer from_json(const QJsonObject &obj);
};

struct Wait {
    size_t ms;
    bool operator==(const Wait &other) const noexcept { return ms == other.ms; }
    static Wait from_json(const QJsonObject &obj);
};

struct RunScript {
    QString interpreter;
    QString script;
    static RunScript from_json(const QJsonObject &obj);
    bool operator==(const RunScript &other) const noexcept {
        return interpreter == other.interpreter && script == other.script;
    }
};

using Bind = std::variant<PressKey, ReleaseKey, SwapLayer, Wait, RunScript>;

inline uint qHash(const PressKey &key, uint seed = 0) noexcept {
    return qHash(static_cast<uint>(key.key_code), seed);
}

inline uint qHash(const ReleaseKey &key, uint seed = 0) noexcept {
    return qHash(static_cast<uint>(key.key_code), seed);
}

inline uint qHash(const SwapLayer &layer, uint seed = 0) noexcept {
    return qHash(static_cast<quintptr>(layer.new_layer), seed);
}

inline uint qHash(const Wait &wait, uint seed = 0) noexcept {
    return qHash(static_cast<quintptr>(wait.ms), seed);
}
inline uint qHash(const RunScript &key, uint seed = 0) noexcept {
    return qHash(key.interpreter, seed) ^ qHash(key.script, seed << 1);
}

inline uint qHash(const Bind &bind, uint seed = 0) noexcept {
    return std::visit([seed](const auto &value) { return qHash(value, seed); },
                      bind);
}

Bind parse_bind(const QJsonObject &obj);

struct SequenceTrigger {
    SequenceBehavior behavior;
    QList<AdvancedTrigger> sequence;
    QList<Bind> binds;

    bool operator==(const SequenceTrigger &other) const noexcept {
        return behavior == other.behavior && sequence == other.sequence &&
               binds == other.binds;
    }
    static SequenceTrigger from_json(const QJsonObject &obj);
};

struct Layer {
    QString layer_name;
    QList<std::pair<BasicTrigger, QList<Bind>>> basic_remappings;
    QList<SequenceTrigger> sequence_remappings;
    static Layer from_json(const QJsonObject &obj);
    bool operator==(const Layer &other) const noexcept {
        return layer_name == other.layer_name &&
               basic_remappings == other.basic_remappings &&
               sequence_remappings == other.sequence_remappings;
    }
};

struct Profile {
    QString name;
    QList<Layer> layers;
    size_t default_layer;
    static Profile from_json(const QJsonObject &obj);
    static Profile from_bytes(const QByteArray &bytes);
    static Profile from_file(const QString &filename);
    static Profile loadLatest();

    bool operator==(const Profile &other) const noexcept {
        return name == other.name && layers == other.layers &&
               default_layer == other.default_layer;
    }
};
