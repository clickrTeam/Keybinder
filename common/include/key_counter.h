#pragma once
#include "key_code.h"
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>
#include <cstdint>
#include <mutex>

class KeyCounter : public QObject {
    Q_OBJECT
  public:
    explicit KeyCounter(const QString &filename = "keycounts.json",
                        QObject *parent = nullptr);
    ~KeyCounter();

    void increment(const KeyCode &key);
    void clear();

    QJsonObject to_json() const;

  private:
    void load();
    void save();

    QString filename;
    QMap<QString, int> counts;
    mutable std::mutex mtx;
    QTimer save_timer;
    bool dirty = true;
};
