#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

const QString LOG_FILE_NAME = "myapp.log";
const QString LOG_DIR = "logs";
const int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
const int MAX_BACKUP_FILES = 5;

class Logger {
  public:
    Logger();
    void logMessage(const QString &msg);
    void cleanUp();

  private:
    void initLogFile();
    void rollOverLogFile();
    QString getBackupFilePath();
    void removeOldestBackupFile();

    QFile logFile_;
};

void myMessageHandler(QtMsgType type, const QMessageLogContext &context,
                      const QString &msg);

#endif // LOGGER_H
