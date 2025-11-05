#include "key_counter.h"
#include "key_code.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>
#include <QtCore/qlogging.h>

KeyCounter::KeyCounter(const QString &filename, QObject *parent)
    : QObject(parent), filename(filename) {

    load();

    // Save every 5 seconds
    connect(&save_timer, &QTimer::timeout, this, &KeyCounter::save);
    save_timer.start(5000);
}

KeyCounter::~KeyCounter() { save(); }

void KeyCounter::increment(const KeyCode &key) {
    qDebug() << "increment: " << str_to_keycode.find_backward(key);
    {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        counts[str_to_keycode.find_backward(key)] += 1;
        dirty = true;
    }
}

void KeyCounter::clear() {
    {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        counts.clear();
        save();
    }
}

QJsonObject KeyCounter::to_json() {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    QJsonObject obj;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        obj[it.key()] = it.value();
    }
    return obj;
}

void KeyCounter::load() {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;

    QJsonObject obj = doc.object();
    counts.clear();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.value().isDouble()) {
            counts[it.key()] = it.value().toInt();
        }
    }
}

void KeyCounter::save() {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    qDebug() << "Saving keycounts to " << filename;
    if (!dirty)
        return;

    if (counts.empty()) {
        QFile::remove(filename);
    } else {
        QJsonDocument doc(to_json());
        QFile file(filename);

        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
        }
    }
    dirty = false;
}
