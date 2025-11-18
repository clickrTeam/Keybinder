// temp_file_manager.cpp
#include "temp_file_manager.h"
#include <QDebug>
#include <QDir>
#include <QTemporaryFile>

QString TempFileManager::get_temp_file(const QString &key,
                                       const QString &content,
                                       const QString &ext) {
    std::lock_guard<std::mutex> locker(mutex);

    if (temp_files.contains(key)) {
        return temp_files[key];
    }

    QString temp_file_name = QDir::tempPath() + "/runscript_XXXXXX";
    if (!ext.isEmpty())
        temp_file_name += ext;

    QTemporaryFile temp_file(temp_file_name);
    temp_file.setAutoRemove(false);

    if (!temp_file.open()) {
        qDebug() << "Failed to open temp file";
        return QString();
    }

    temp_file.write(content.toUtf8());
    temp_file.flush();
    QString path = temp_file.fileName();
    temp_file.close();

    temp_files[key] = path;
    return path;
}
