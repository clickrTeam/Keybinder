#pragma once
#include <QJsonObject>
#include <QString>
#include <mutex>

class KeybinderSettings {
  public:
    explicit KeybinderSettings(const QString &filename = "settings.json");

    void load();
    void save();

    QJsonObject to_json();
    bool load_from_json(const QJsonObject &obj);

    bool get_log_key_frequency();

  private:
    QString filename;
    std::mutex mutex;
    bool log_key_frequency;
};
