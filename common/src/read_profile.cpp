#include "read_profile.h"
#include <QFileInfo>
#include <QtLogging>
#include <cstddef>

#include "key_code.h"
#include "profile.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <optional>
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
MinimumWait MinimumWait::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "duration"});
    return MinimumWait{
        static_cast<size_t>(get_property_as_number(obj, "duration"))};
}

MaximumWait MaximumWait::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "duration"});
    return MaximumWait{
        static_cast<size_t>(get_property_as_number(obj, "duration"))};
}

SequenceBehavior parse_behavior(const QString &str) {
    if (str == "capture") {
        return SequenceBehavior::Capture;
    } else if (str == "release") {
        return SequenceBehavior::Release;
    } else if (str == "default") {
        return SequenceBehavior::Default;
    } else {
        throw std::invalid_argument(
            ("SequenceBehavior Behavior: " + str.toStdString()).c_str());
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

// SwapLayer
SwapLayer SwapLayer::from_json(const QJsonObject &obj) {

    warn_extra_properties(obj, {"type", "value"});
    return SwapLayer{(size_t)get_property_as_number(obj, "value")};
}
Wait Wait::from_json(const QJsonObject &obj) {

    warn_extra_properties(obj, {"type", "duration"});
    return Wait{(size_t)get_property_as_number(obj, "duration")};
}

RunScript RunScript::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "interpreter", "script"});
    return RunScript{
        get_property_as_string(obj, "interpreter"),
        get_property_as_string(obj, "script"),
    };
}

AppTrigger AppTrigger::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "appName"});
    return AppTrigger{
        get_property_as_string(obj, "appName")
    };
}

AppLaunch AppLaunch::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"type", "appName"});
    return AppLaunch{
        get_property_as_string(obj, "appName"),
    };
}

BasicTrigger parse_basic_trigger(const QJsonObject &obj) {
    QString trigger_type = get_property_as_string(obj, "type");

    if (trigger_type == "key_press") {
        return KeyPress::from_json(obj);
    } else if (trigger_type == "key_release") {
        return KeyRelease::from_json(obj);
    } else if (trigger_type == "app_launch") {
        return AppTrigger::from_json(obj);
    }
    throw std::invalid_argument(
        ("Invalid trigger type: " + trigger_type.toStdString()).c_str());
}

AdvancedTrigger parse_advanced_trigger(const QJsonObject &obj) {
    QString trigger_type = get_property_as_string(obj, "type");

    if (trigger_type == "key_press") {
        return KeyPress::from_json(obj);
    } else if (trigger_type == "key_release") {
        return KeyRelease::from_json(obj);
    } else if (trigger_type == "minimum_wait") {
        return MinimumWait::from_json(obj);
    } else if (trigger_type == "maximum_wait") {
        return MaximumWait::from_json(obj);
    } else if (trigger_type == "app_launch") {
        return AppTrigger::from_json(obj);
    }
    throw std::invalid_argument(
        ("Invalid trigger type: " + trigger_type.toStdString()).c_str());
}
QList<Bind> parse_binds(const QJsonArray &json_arr) {

    QList<Bind> binds;
    for (const QJsonValue &bind : json_arr) {
        auto bind_obj = get_value_as_object(bind);

        QString bind_type = get_property_as_string(bind_obj, "type");
        Bind bind_v;

        if (bind_type == "press_key") {
            bind_v = PressKey::from_json(bind_obj);
        } else if (bind_type == "release_key") {
            bind_v = ReleaseKey::from_json(bind_obj);
        } else if (bind_type == "switch_layer") {
            bind_v = SwapLayer::from_json(bind_obj);
        } else if (bind_type == "wait") {
            bind_v = Wait::from_json(bind_obj);
        } else if (bind_type == "run_script") {
            bind_v = RunScript::from_json(bind_obj);
        } else if (bind_type == "launch_app") {
            bind_v = AppLaunch::from_json(bind_obj);
        } else {
            throw std::invalid_argument(
                ("Invalid bind type: " + bind_type.toStdString()).c_str());
        }
        binds.push_back(bind_v);
    }

    return binds;
}

std::pair<BasicTrigger, QList<Bind>>
parse_basic_remapping(const QJsonObject &obj) {
    warn_extra_properties(obj, {"trigger", "binds"});
    BasicTrigger trigger =
        parse_basic_trigger(get_property_as_object(obj, "trigger"));
    auto binds = parse_binds(get_property_as_array(obj, "binds"));
    return std::make_pair(trigger, binds);
}
SequenceTrigger SequenceTrigger::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"triggers", "binds", "behavior"});
    QList<AdvancedTrigger> triggers;
    for (const QJsonValue &trigger_json :
         get_property_as_array(obj, "triggers")) {
        triggers.push_back(
            parse_advanced_trigger(get_value_as_object(trigger_json)));
    }
    auto binds = parse_binds(get_property_as_array(obj, "binds"));
    auto behavior = parse_behavior(get_property_as_string(obj, "behavior"));

    return SequenceTrigger{
        .behavior = behavior,
        .sequence = triggers,
        .binds = binds,
    };
}

Layer Layer::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"layer_name", "remappings"});
    QString layer_name = get_property_as_string(obj, "layer_name");

    QList<std::pair<BasicTrigger, QList<Bind>>> basic_remappings;
    QList<SequenceTrigger> sequence_remappings;
    for (const QJsonValue &remapping :
         get_property_as_array(obj, "remappings")) {
        auto obj = get_value_as_object(remapping);
        if (obj.contains("triggers")) {
            sequence_remappings.push_back(SequenceTrigger::from_json(obj));
        } else if (obj.contains("trigger")) {
            basic_remappings.push_back(parse_basic_remapping(obj));
        } else {
        }
    }
    return Layer{.layer_name = layer_name,
                 .basic_remappings = basic_remappings,
                 .sequence_remappings = sequence_remappings};
}
// Profile
Profile Profile::default_profile() {
    return Profile{
        .name = "default",
        .layers = {Layer{
            .layer_name = "default",
            .basic_remappings = {},
            .sequence_remappings = {},
        }},
        .default_layer = 0,

    };
}

Profile Profile::from_json(const QJsonObject &obj) {
    warn_extra_properties(obj, {"profile_name", "default_layer", "layers"});
    QString profile_name = get_property_as_string(obj, "profile_name");
    qDebug() << "Loading PROFILE_NAME:" << profile_name;
    size_t default_layer = (size_t)get_property_as_number(obj, "default_layer");

    QList<Layer> layers;

    for (const QJsonValue &remapping : get_property_as_array(obj, "layers")) {
        layers.push_back(Layer::from_json(get_value_as_object(remapping)));
    }
    qDebug() << "Loaded PROFILE_NAME:" << profile_name;
    saveLatestJsonProfile(obj);

    return Profile{
        .name = profile_name,
        .layers = layers,
        .default_layer = default_layer,
    };
}

Profile Profile::from_bytes(const QByteArray &bytes) {
    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if (doc.isNull() || !doc.isObject()) {
        qCritical() << "Invalid JSON!";
        throw std::invalid_argument("Invalid JSON document");
    }

    return Profile::from_json(doc.object());
}

std::optional<Profile> Profile::from_file(const QString &filename) {
    QFile file(filename);

    if (!file.exists()) {
        qCritical() << "Profile does not exist:" << file.fileName();
        return std::nullopt;
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

std::optional<Profile> Profile::load_latest() {
    qDebug() << "Loading latest profile";
    return Profile::from_file(LATEST_PROFILE_FILE_LOCATION);
}
