#include "logger.h"
#include <QLoggingCategory>

// hacky global varible.
bool firstIntialize = true;

// Is more of a Logger Factory, since logger is re-constructed on every logger.
Logger::Logger() { initLogFile(); }

void Logger::logMessage(const QString &msg) {
    if (logFile_.size() > MAX_FILE_SIZE) {
        rollOverLogFile();
    }

    QTextStream stream(&logFile_);
    stream << msg << Qt::endl;
    logFile_.flush();
}

void Logger::initLogFile() {
    QDir dir(LOG_DIR);
    if (!dir.exists()) {
        dir.mkpath(LOG_DIR);
    }

    logFile_.setFileName(LOG_DIR + "/" + LOG_FILE_NAME);
    logFile_.open(QFile::WriteOnly | QFile::Text | QFile::Append);

    if (firstIntialize) {
        firstIntialize = false;
        this->logMessage("/0|----------------------------------START-OF-"
                         "PROGRAM----------------------------------|0\\");
    }
}

void Logger::cleanUp() {
    this->logMessage("\\0|-----------------------------------END--OF-PROGRAM---"
                     "-------------------------------|0/");
}

void Logger::rollOverLogFile() {
    logFile_.close();

    // Rename the current log file to a backup file
    QString logFilePath = logFile_.fileName();
    QString backupFilePath = getBackupFilePath();

    // Remove the oldest backup file if there are too many
    removeOldestBackupFile();

    QFile::rename(logFilePath, backupFilePath);

    initLogFile();
}

QString Logger::getBackupFilePath() {
    QString timestamp =
        QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString backupFilePath =
        LOG_DIR + "/" + LOG_FILE_NAME + "_" + timestamp + ".bak";
    return backupFilePath;
}

void Logger::removeOldestBackupFile() {
    QDir dir(LOG_DIR);
    QStringList backupFiles =
        dir.entryList(QStringList(LOG_FILE_NAME + "_*.bak"));
    if (backupFiles.size() >= MAX_BACKUP_FILES) {
        // Sort the backup files by timestamp
        backupFiles.sort();

        // Remove the oldest backup file
        QString oldestBackupFile = LOG_DIR + "/" + backupFiles.first();
        QFile::remove(oldestBackupFile);
    }
}

void myMessageHandler(QtMsgType type, const QMessageLogContext &context,
                      const QString &msg) {
    QString message =
        QString("%1 %2 %3")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(type == QtDebugMsg      ? "DEBUG"
                 : type == QtInfoMsg     ? "INFO"
                 : type == QtWarningMsg  ? "WARNING"
                 : type == QtCriticalMsg ? "CRITICAL"
                                         : "FATAL")
            .arg(msg);
    Logger logger;
    logger.logMessage(message);
}
