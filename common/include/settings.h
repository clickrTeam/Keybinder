#pragma once
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <mutex>

class KeybinderSettings : public QObject {
    Q_OBJECT
  public:
    explicit KeybinderSettings(const QString &filename = "settings.json");

    void load();
    void save(QJsonDocument doc, QString filename);

    QJsonObject to_json();
    bool load_from_json(const QJsonObject &obj);

    bool get_log_key_frequency();
  signals:
    void settingsChanged();

  private:
    QString filename;
    std::mutex mutex;
    bool log_key_frequency;
};
