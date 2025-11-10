#pragma once
#include <QString>
#include <QJsonObject>
#include <QMutex>

class AppCache {
  public:
    struct Entry { QString path; QString appId; };

    AppCache();
    bool lookup(const QString &name, Entry &out);
    void store(const QString &name, const Entry &entry);

  private:
    void load();
    QString cacheFile;
    QJsonObject obj;
    QMutex m;
};
