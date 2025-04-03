#ifndef READPROFILE_H
#define READPROFILE_H
#include "profile.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

// Define constants for each JSON key
#define PROFILE_NAME "name"
#define PROFILE_LAYERS "layers"
#define LAYER_NAME "layer_name"
#define LAYER_KEYBINDS "keybinds"
#define KEYBIND_KEY "key"
#define KEYBIND_BIND "bind"
#define VALUE "value"
#define TYPE "type"

#define TAP "tap"

// functions
Profile proccessProfile(const QString &profileFilePath);

Profile readProfile(QJsonObject profile);
Layer readLayer(QJsonObject layer);
Key readKey(QJsonObject key);
Bind readBind(QJsonObject bind);

#endif // READPROFILE_H
