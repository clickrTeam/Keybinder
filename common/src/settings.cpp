#include "settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <mutex>

KeybinderSettings::KeybinderSettings(const QString &filename)
    : filename(filename), log_key_frequency(false) {
    load();
}

void KeybinderSettings::load() {
    std::lock_guard<std::mutex> lock(mutex);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;

    load_from_json(doc.object());
}

void KeybinderSettings::save() {
    std::lock_guard<std::mutex> lock(mutex);

    QJsonDocument doc(to_json());

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

QJsonObject KeybinderSettings::to_json() {
    std::lock_guard<std::mutex> lock(mutex);

    QJsonObject obj;
    obj["log_key_frequency"] = log_key_frequency;
    return obj;
}

bool KeybinderSettings::load_from_json(const QJsonObject &obj) {
    std::lock_guard<std::mutex> lock(mutex);
    bool successful = false;
    if (obj.contains("log_key_frequency") &&
        obj["log_key_frequency"].isBool()) {
        log_key_frequency = obj["log_key_frequency"].toBool();
        successful = true;
    }
    save();
    return successful;
}

bool KeybinderSettings::get_log_key_frequency() {
    std::lock_guard<std::mutex> lock(mutex);
    return log_key_frequency;
}
