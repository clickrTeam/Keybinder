#include "settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtCore/qlogging.h>
#include <mutex>

KeybinderSettings::KeybinderSettings(const QString &filename)
    : filename(filename), log_key_frequency(false) {
    load();
}

void KeybinderSettings::load() {
    QJsonDocument doc;
    {
        std::lock_guard<std::mutex> lock(mutex);
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QByteArray data = file.readAll();
        file.close();

        QJsonParseError err;
        doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject())
            return;
    }

    load_from_json(doc.object());
}

void KeybinderSettings::save(QJsonDocument doc, QString filename) {
    std::lock_guard<std::mutex> lock(mutex);

    qDebug() << "Saving settings to '" << filename << "'";
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
    qDebug() << "Done saving settings to '" << filename << "'";
}

QJsonObject KeybinderSettings::to_json() {
    std::lock_guard<std::mutex> lock(mutex);

    QJsonObject obj;
    obj["log_key_frequency"] = log_key_frequency;
    return obj;
}

bool KeybinderSettings::load_from_json(const QJsonObject &obj) {
    bool successful = false;
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (obj.contains("log_key_frequency") &&
            obj["log_key_frequency"].isBool()) {
            log_key_frequency = obj["log_key_frequency"].toBool();
            successful = true;
        }
    }

    QJsonObject settings_json = to_json();
    save(QJsonDocument(settings_json), filename);
    emit settingsChanged();
    return successful;
}

bool KeybinderSettings::get_log_key_frequency() {
    std::lock_guard<std::mutex> lock(mutex);
    qDebug() << "for get do log " << log_key_frequency;
    return log_key_frequency;
}
