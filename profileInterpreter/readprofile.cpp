#include "readprofile.h"
#include <qdebug.h>
#include <QFileInfo>

void proccessProfile(const QString &profileFilePath) {
    QFile profileFile(profileFilePath);

    // Get absolute path
    QFileInfo fileInfo(profileFilePath);
    qDebug() << "Checking file at:" << fileInfo.absoluteFilePath();

    if (!profileFile.exists()) {
        qCritical() << "Profile does not exist:" << profileFile.fileName();
        return;
    }

    qDebug() << "Found profile";
    if (!profileFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open file!";
        return;
    }

    QByteArray jsonData = profileFile.readAll();
    profileFile.close();

    // Parse JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qCritical() << "Invalid JSON!";
        return;
    }
    qDebug() << "Profile json read";
}

void readProfile(QFile profileFile) {}
