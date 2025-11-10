#include "app_cache.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QMutexLocker>

AppCache::AppCache() {
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(base);
    cacheFile = QDir(base).filePath("app_launcher_cache.json");
    load();
}

void AppCache::load() {
    QFile f(cacheFile);
    if (!f.exists()) return;
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) obj = doc.object();
}

bool AppCache::lookup(const QString &name, Entry &out) {
    QMutexLocker l(&m);
    if (obj.contains(name)) {
        QJsonObject e = obj.value(name).toObject();
        out.path = e.value("path").toString();
        out.appId = e.value("appId").toString();
        return true;
    }
    return false;
}

void AppCache::store(const QString &name, const Entry &entry) {
    {
        QMutexLocker l(&m);
        QJsonObject e;
        e.insert("path", entry.path);
        e.insert("appId", entry.appId);
        obj.insert(name, e);
    }
    QString tmp = cacheFile + ".tmp";
    QFile f(tmp);
    if (f.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(obj);
        f.write(doc.toJson(QJsonDocument::Compact));
        f.close();
        QFile::remove(cacheFile);
        QFile::rename(tmp, cacheFile);
    }
}
