#ifndef READPROFILE_H
#define READPROFILE_H
#include <QFile>
#include <QDebug>
#include <qjsondocument.h>

void proccessProfile(const QString &profileFilePath);

void readProfile(QFile profileFile);

#endif // READPROFILE_H
