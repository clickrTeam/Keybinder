// temp_file_manager.h
#pragma once

#include <QHash>
#include <QString>
#include <mutex>

class TempFileManager {
  public:
    TempFileManager() = default;

    QString get_temp_file(const QString &key, const QString &content,
                          const QString &ext = QString());

  private:
    std::mutex mutex;
    QHash<QString, QString> temp_files;
};
