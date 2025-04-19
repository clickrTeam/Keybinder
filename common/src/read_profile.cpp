#include "read_profile.h"

#include <QFileInfo>
#include <QtLogging>
#include <cstddef>

#include "profile.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <stdexcept>
#include <utility>

// Helper method to make parsing less repetative
QJsonObject get_value_as_object(const QJsonValue &value) {
    if (!value.isObject()) {
        throw std::invalid_argument("expected object");
    }

    return value.toObject();
}

QJsonObject get_property_as_object(const QJsonObject &obj, const QString &key) {
    if (!obj.contains(key)) {
        throw std::invalid_argument(
            ("Invalid or missing '" + key.toStdString() + "' in JSON.")
                .c_str());
    }

    QJsonValue value = obj[key];
    if (!value.isObject()) {
        throw std::invalid_argument(
            ("The property '" + key.toStdString() + "' is not an object.")
                .c_str());
    }

    return value.toObject();
}

QString get_property_as_string(const QJsonObject &obj, const QString &key) {
    if (!obj.contains(key)) {
        throw std::invalid_argument(
            ("Invalid or missing '" + key.toStdString() + "' in JSON.")
                .c_str());
    }

    QJsonValue value = obj[key];
    if (!value.isString()) {
        throw std::invalid_argument(
            ("The property '" + key.toStdString() + "' is not a string.")
                .c_str());
    }

    return value.toString();
}

QJsonArray get_property_as_array(const QJsonObject &obj, const QString &key) {
    if (!obj.contains(key)) {
        throw std::invalid_argument(
            ("Invalid or missing '" + key.toStdString() + "' in JSON.")
                .c_str());
    }

    QJsonValue value = obj[key];
    if (!value.isArray()) {
        throw std::invalid_argument(
            ("The property '" + key.toStdString() + "' is not an array.")
                .c_str());
    }

    return value.toArray();
}

double get_property_as_number(const QJsonObject &obj, const QString &key) {
    if (!obj.contains(key)) {
        throw std::invalid_argument(
            ("Invalid or missing '" + key.toStdString() + "' in JSON.")
                .c_str());
    }

    QJsonValue value = obj[key];
    if (!value.isDouble()) {
        throw std::invalid_argument(
            ("The property '" + key.toStdString() + "' is not a number.")
                .c_str());
    }

    return value.toDouble();
}

void warn_extra_properties(const QJsonObject &obj,
                           std::initializer_list<QString> valid_keys) {
    for (const QString &key : obj.keys()) {

        bool is_known_key = false;
        // Check if the key is in the validKeys span
        for (const QString &valid_key :
             valid_keys) { // Use reference to avoid copy
            if (key == valid_key) {
                is_known_key = true;
                break;
            }
        }

        // Print a warning if the key is not in the valid keys
        if (!is_known_key) {
            qWarning() << "Warning: Extra property found in JSON object: "
                       << key;
        }
    }
}

KeyCode str_to_keycode(const QString &str) {
    if (!string_to_key_code.contains(str)) {
        throw std::invalid_argument(
            ("The string '" + str.toStdString() + "' is not a valid key.")
                .c_str());
    }

    return string_to_key_code[str];
};

// KeyPress
KeyPress KeyPress::from_json(const QJsonObject &obj) {

    warn_extra_properties(obj, {"type", "value"});
    return KeyPress{str_to_keycode(get_property_as_string(obj, "value"))};
}

// KeyRelease
KeyRelease KeyRelease::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "value"});
    return KeyRelease{str_to_keycode(get_property_as_string(obj, "value"))};
}

// TapSequence
TapSequence TapSequence::from_json(const QJsonObject &obj) {
    // TODO: not sure exacty what this looks like
    return TapSequence{};
}

TimedTriggerBehavior parse_behavior(const QString &str) {
    if (str == "capture") {
        return TimedTriggerBehavior::Capture;
    } else if (str == "release") {
        return TimedTriggerBehavior::Release;
    } else if (str == "default") {
        return TimedTriggerBehavior::Default;
    } else {
        throw std::invalid_argument(
            ("Invalid Timed Bind Behavior: " + str.toStdString()).c_str());
    }
}

PressKey PressKey::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "value"});
    return PressKey{str_to_keycode(get_property_as_string(obj, "value"))};
}

// ReleaseKey
ReleaseKey ReleaseKey::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "value"});
    return ReleaseKey{str_to_keycode(get_property_as_string(obj, "value"))};
}

// TapKey
TapKey TapKey::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "value"});
    return TapKey{str_to_keycode(get_property_as_string(obj, "value"))};
}

// SwapLayer
SwapLayer SwapLayer::from_json(const QJsonObject &obj) {

    warn_extra_properties(obj, {"type", "layer_num"});
    return SwapLayer{(size_t)get_property_as_number(obj, "layer_num")};
}

Trigger parse_trigger(const QJsonObject &obj) {
    QString trigger_type = get_property_as_string(obj, "type");

    if (trigger_type == "key_press") {
        return KeyPress::from_json(obj);
    } else if (trigger_type == "key_release") {
        return KeyRelease::from_json(obj);
    } else if (trigger_type == "tap_sequence") {
        return TapSequence::from_json(obj);
    }

    throw std::invalid_argument(
        ("Invalid trigger type: " + trigger_type.toStdString()).c_str());
}
// using Bind = std::variant<PressKey, ReleaseKey, TapKey, SwapLayer>;
Bind parse_bind(const QJsonObject &obj) {
    QString bind_type = get_property_as_string(obj, "type");

    if (bind_type == "press_key") {
        return PressKey::from_json(obj);
    } else if (bind_type == "release_key") {
        return ReleaseKey::from_json(obj);
    } else if (bind_type == "tap_key") {
        return TapKey::from_json(obj);
    } else if (bind_type == "swap_layer") {
        return SwapLayer::from_json(obj);
    }

    throw std::invalid_argument(
        ("Invalid bind type: " + bind_type.toStdString()).c_str());
}

std::pair<Trigger, Bind> parse_remapping(const QJsonObject &obj) {
    warn_extra_properties(obj, {"trigger", "bind"});
    Trigger trigger = parse_trigger(get_property_as_object(obj, "trigger"));
    Bind bind = parse_bind(get_property_as_object(obj, "bind"));
    return std::make_pair(trigger, bind);
}

Layer Layer::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"layer_name", "layer_number", "remappings"});
    QString layer_name = get_property_as_string(obj, "layer_name");
    QList<std::pair<Trigger, Bind>> remappings;
    for (const QJsonValue &remapping :
         get_property_as_array(obj, "remappings")) {
        remappings.push_back(parse_remapping(get_value_as_object(remapping)));
    }
    return Layer{.layer_name = layer_name, .remappings = remappings};
}

// Profile
Profile Profile::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"profile_name", "default_layer", "layers"});
    QString profile_name = get_property_as_string(obj, "profile_name");
    // TODO: check cast
    size_t default_layer = (size_t)get_property_as_number(obj, "default_layer");
    QList<Layer> layers;

    for (const QJsonValue &remapping : get_property_as_array(obj, "layers")) {
        layers.push_back(Layer::from_json(get_value_as_object(remapping)));
    }

    return Profile{
        .name = profile_name, .layers = layers, .default_layer = default_layer};
}

Profile Profile::from_bytes(const QByteArray &bytes) {
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if (doc.isNull() || !doc.isObject()) {
        qCritical() << "Invalid JSON!";
        throw std::invalid_argument("Invalid JSON document");
    }

    return Profile::from_json(doc.object());
}

Profile Profile::from_file(const QString &filename) {

    QFile file(filename);

    // Get absolute path
    qDebug() << "Checking file at:" << filename;

    if (!file.exists()) {
        qCritical() << "Profile does not exist:" << file.fileName();
        throw std::invalid_argument("File does not exist");
    }

    qDebug() << "Found profile";
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open file!";
        throw std::invalid_argument("Could not open file");
    }

    QByteArray json_data = file.readAll();
    return Profile::from_bytes(json_data);
}
