#pragma once

#include <QHash>
#include <QString>
#include <mutex>

class TempFileManager {
  public:
    explicit TempFileManager(const QString &extension);

    // Returns the path to a temp file containing ''
    QString get_temp_file(const QString &key, const QString &content);

  private:
    QString extension;
    std::mutex mutex;
    QHash<QString, QString> temp_files;
};
