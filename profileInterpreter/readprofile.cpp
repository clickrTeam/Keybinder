#include "readprofile.h"
#include <qdebug.h>
#include <QFileInfo>

// TODO add json verification and creation on electron app.
Profile proccessProfile(const QString &profileFilePath) {
    QFile profileFile(profileFilePath);

    // Get absolute path
    QFileInfo fileInfo(profileFilePath);
    qDebug() << "Checking file at:" << fileInfo.absoluteFilePath();

    if (!profileFile.exists()) {
        qCritical() << "Profile does not exist:" << profileFile.fileName();
        return Profile();
    }

    qDebug() << "Found profile";
    if (!profileFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open file!";
        return Profile();
    }

    QByteArray jsonData = profileFile.readAll();
    profileFile.close();

    // Parse JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qCritical() << "Invalid JSON!";
        return Profile();
    }
    qDebug() << "Profile json read";

    return readProfile(jsonDoc.object());
}

Profile readProfile(QJsonObject profile) {
    Profile pro;
    qDebug() << "Profile Name:" << profile.value(PROFILE_NAME).toString();
    pro.name = profile.value(PROFILE_NAME).toString();

    const QJsonArray layers = profile.value(PROFILE_LAYERS).toArray();
    for (const QJsonValue& layer : layers) {
        pro.layers.append(readLayer(layer.toObject()));
    }
    pro.isNull = false;
    return pro;
}

Layer readLayer(QJsonObject layer) {
    Layer lyr;
    qDebug() << "Layer Name:" << layer.value(LAYER_NAME).toString();
    lyr.layerName = layer.value(LAYER_NAME).toString();

    const QJsonArray keybinds = layer.value(LAYER_KEYBINDS).toArray();
    for (const QJsonValue& keybindValue : keybinds) {
        // Foreach keybind
        QJsonObject keybind = keybindValue.toObject();
        QJsonObject key = keybind.value(KEYBIND_KEY).toObject();
        QString keyType = key.value(TYPE).toString();
        QJsonObject bind = keybind.value(KEYBIND_BIND).toObject();

        if (keyType == TAP) {
            qDebug() << "TapKey" << key;

            int keyVk = stringToKey(key.value(VALUE).toString());
            int bindVk = stringToKey(bind.value(VALUE).toString());

            lyr.tapKeyBinds[keyVk] = bindVk;
        } else if (keyType == TIMED) {
            qDebug() << "TimedKey" << key;

            TimedKeyBind timedKey;
            QJsonArray keyTimePairs = key.value(KEY_TIME_PAIRS).toArray();
            // qDebug() << "JkeyTimePairS" << keyTimePairs;
            timedKey.capture = key.value(CAPTURE).toBool();
            timedKey.release = key.value(RELEASE).toBool();

            for (const QJsonValue &JkeyTimePairValue : keyTimePairs) {
                // Foreach key & time pair.
                QJsonObject JkeyTimePair = JkeyTimePairValue.toObject();
                // qDebug() << "JkeyTimePair" << JkeyTimePair;
                TimedKeyBind::KeyTimePair keyTimePair;

                keyTimePair.keyVk = stringToKey(JkeyTimePair.value(VALUE).toString());
                keyTimePair.time = JkeyTimePair.value(TIME).toInt(100);

                timedKey.keyTimePairs.append(keyTimePair);
            }

            timedKey.bind = stringToKey(bind.value(VALUE).toString());
            lyr.timedKeyBinds[timedKey.keyTimePairs.first().keyVk] = timedKey;
        }
    }
    qDebug() << "Tap keybinds:" << lyr.tapKeyBinds.count();
    qDebug() << "Timed keybinds:" << lyr.timedKeyBinds.count();
    return lyr;
}

int stringToKey(QString keyString) {
    // Look up the key string in the QMap
    if (keyMap.contains(keyString)) {
        return keyMap.value(keyString);
    }
    qCritical() << "KeyMapWin missing following key:" << keyString;
    return 0;  // Return 0 if the key is not found in the map
}

// {
//     "name": "Default Profile",
//              "layers": [
//                             {
//                                 "layer_name": "Gaming Layer",
//                                 "keybinds": [
//                                     {
//                                         "key": {
//                                             "value": "w",
//                                             "type": "tap"
//                                         },
//                                         "bind": {
//                                             "value": "q",
//                                             "type": "tap"
//                                         }
//                                     }
//                          ]
//              }
//     ]
// }
