#pragma once
#include "key_map.h"
#include "profile.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <QFileInfo>
#include <QtLogging>

const QString LATEST_PROFILE_FILE_LOCATION = "./last_loaded.profile.json";
void saveLatestJsonProfile(const QJsonObject &obj);
