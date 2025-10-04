#include "read_profile.h"

#include <QFileInfo>
#include <QtLogging>
#include <cstddef>

#include "key_code.h"
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

QJsonArray get_value_as_array(const QJsonValue &value) {
    if (!value.isArray()) {
        throw std::invalid_argument("expected array");
    }

    return value.toArray();
}

QString get_value_as_string(const QJsonValue &value) {
    if (!value.isString()) {
        throw std::invalid_argument("expected string");
    }

    return value.toString();
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
KeyCode parse_key(const QString &key) {
    if (str_to_keycode.contains_forward(key)) {
        return str_to_keycode.find_forward(key);
    } else {
        throw std::invalid_argument(
            ("Unkown key:" + key.toStdString()).c_str());
    }
}

// KeyPress
KeyPress KeyPress::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "value"});
    if (!str_to_keycode.contains_forward(get_property_as_string(obj, "value")))
        qCritical() << "Key missing from forward map:"
                    << get_property_as_string(obj, "value");
    return KeyPress{parse_key(get_property_as_string(obj, "value"))};
}

// KeyRelease
KeyRelease KeyRelease::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "value"});
    if (!str_to_keycode.contains_forward(get_property_as_string(obj, "value")))
        qCritical() << "Key missing from forward map:"
                    << get_property_as_string(obj, "value");
    return KeyRelease{parse_key(get_property_as_string(obj, "value"))};
}

//   "type": "tap_sequence",
//   "key_time_pairs": [
//     [
//       "Q",
//       300
//     ],
//     [
//       "Q",
//       300
//     ]
//   ],
//   "behavior": "default"
// },
// TapSequence
TapSequence TapSequence::from_json(const QJsonObject &obj) {
    QList<KeyCode> key_sequence;
    warn_extra_properties(obj, {"type", "key_time_pairs", "behavior"});
    for (const QJsonValue &val : get_property_as_array(obj, "key_time_pairs")) {
        auto pair = get_value_as_array(val);
        if (!str_to_keycode.contains_forward(get_value_as_string(pair.at(0))))
            qCritical() << "Key missing from forward map:"
                        << get_value_as_string(pair.at(0));
        key_sequence.push_back(parse_key(get_value_as_string(pair.at(0))));
        // TODO: use the timeout which is the second element in this array
    }

    auto behavior = parse_behavior(get_property_as_string(obj, "behavior"));

    return TapSequence{key_sequence, behavior};
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
    if (!str_to_keycode.contains_forward(get_property_as_string(obj, "value")))
        qCritical() << "Key missing from forward map:"
                    << get_property_as_string(obj, "value");
    return PressKey{parse_key(get_property_as_string(obj, "value"))};
}

// ReleaseKey
ReleaseKey ReleaseKey::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "value"});
    if (!str_to_keycode.contains_forward(get_property_as_string(obj, "value")))
        qCritical() << "Key missing from forward map:"
                    << get_property_as_string(obj, "value");
    return ReleaseKey{parse_key(get_property_as_string(obj, "value"))};
}

// TapKey
TapKey TapKey::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "value"});
    if (!str_to_keycode.contains_forward(get_property_as_string(obj, "value")))
        qCritical() << "Key missing from forward map:"
                    << get_property_as_string(obj, "value");
    return TapKey{parse_key(get_property_as_string(obj, "value"))};
}

// SwapLayer
SwapLayer SwapLayer::from_json(const QJsonObject &obj) {

    warn_extra_properties(obj, {"type", "value"});
    return SwapLayer{(size_t)get_property_as_number(obj, "value")};
}

// Macro
Macro Macro::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "binds"});
    QList<Bind> binds;
    for (const QJsonValue &bindObj : get_property_as_array(obj, "binds")) {
        binds.push_back(parse_bind(get_value_as_object(bindObj)));
    }
    return Macro{.binds = binds};
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
    } else if (bind_type == "switch_layer") {
        return SwapLayer::from_json(obj);
    } else if (bind_type == "macro") {
        return Macro::from_json(obj);
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
    qDebug() << "Loading PROFILE_NAME:" << profile_name;
    // TODO: probably should have some kind of default layer beyond just always
    // the first size_t default_layer = (size_t)get_property_as_number(obj,
    // "default_layer");
    size_t default_layer = 0;
    QList<Layer> layers;

    for (const QJsonValue &remapping : get_property_as_array(obj, "layers")) {
        layers.push_back(Layer::from_json(get_value_as_object(remapping)));
    }
    qDebug() << "Loaded PROFILE_NAME:" << profile_name;
    saveLatestJsonProfile(obj);

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

QJsonObject defaultProfile() {
    QJsonObject profile;
    profile["profile_name"] = "EMPTY-STARTUP-PROFILE";
    profile["layer_count"] = 1;

    QJsonArray layers;
    QJsonObject layer;
    layer["layer_name"] = "default";
    layer["layer_number"] = 0;
    QJsonArray remappings;
    layer["remappings"] = remappings;
    layers.append(layer);

    profile["layers"] = layers;

    return profile;
}

Profile Profile::from_file(const QString &filename) {
    QFile file(filename);

    // Get absolute path
    qDebug() << "Checking file at:" << filename;
    if (filename == "empty" ||
        (!file.exists() && filename == LATEST_PROFILE_FILE_LOCATION)) {
        qDebug() << "Using empty json mapping. Intended for startup by "
                    "electron app.";
        QJsonObject defaultJson = defaultProfile();
        QJsonDocument jsonDoc(defaultJson);
        QByteArray json_data = jsonDoc.toJson();
        return Profile::from_bytes(json_data);
    }

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

void saveLatestJsonProfile(const QJsonObject &obj) {
    QFile file(LATEST_PROFILE_FILE_LOCATION);
    if (!file.open(QFile::WriteOnly)) {
        qCritical() << "Failed to save latest JSON profile: Unable to open file"
                    << LATEST_PROFILE_FILE_LOCATION
                    << " for writing. Error: " << file.errorString();
        return;
    }

    QJsonDocument doc(obj);
    if (file.write(doc.toJson()) == -1) {
        qCritical()
            << "Failed to save latest JSON profile: Unable to write to file"
            << LATEST_PROFILE_FILE_LOCATION << ". Error:" << file.errorString();
        file.close();
        return;
    }

    file.close();
    qDebug() << "Latest JSON profile saved successfully to"
             << LATEST_PROFILE_FILE_LOCATION;
}

Profile Profile::loadLatest() {
    qDebug() << "Loading latest profile";
    return Profile::from_file(LATEST_PROFILE_FILE_LOCATION);
}
