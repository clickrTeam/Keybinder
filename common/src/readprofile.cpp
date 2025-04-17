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
    qDebug() << "Layer:" << layer[LAYER_NAME].toString();

    QJsonArray remappings = layer[LAYER_KEYBINDS].toArray();
    for (const QJsonValue& remapVal : remappings) {
        lyr.keybinds.append(parseRemapping(remapVal.toObject()));
    }
    foreach (Trigger trigger, lyr.keybinds) {
        if (trigger.type == TRIGGERTYPE_LINK) {
            lyr.tapKeyBinds[*trigger.vk] = *trigger.bind.vk;
        }
    }
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

Bind readBind(const QJsonObject& remapping, QString bind_key) {
    Bind bind;
    if (bind_key == BINDTYPE_LINK) {
        qDebug() << "  Link Trigger Value:" << remapping[bind_key].toObject()[VALUE].toString();
        bind = Bind{
            .type = bind_key,
            .vk = stringToKey(remapping[bind_key].toObject()[VALUE].toString())
        };
    } else if (bind_key == BINDTYPE_COMBO) {
        QJsonArray combo = remapping[bind_key].toArray();
        for (const QJsonValue& val : combo)
            qDebug() << "  Combo Key:" << val.toString();
    } else if (bind_key == BINDTYPE_MACRO) {
        QJsonArray macro = remapping[bind_key].toArray();
        for (const QJsonValue& step : macro) {
            QJsonObject obj = step.toObject();
            if (obj.contains(BINDTYPE_LINK)) {
                qDebug() << "  Macro - Link:" << obj[BINDTYPE_LINK].toObject()[VALUE].toString();
            } else if (obj.contains(BINDTYPE_COMBO)) {
                QJsonArray combo = obj[BINDTYPE_COMBO].toArray();
                qDebug() << "  Macro - Combo:";
                for (const QJsonValue& k : combo)
                    qDebug() << "   -" << k.toString();
            }
        }
    } else if (bind_key == BINDTYPE_TIMEDMACRO) {
        QJsonArray macro = remapping[bind_key].toArray();
        for (const QJsonValue& step : macro) {
            QJsonObject bind = step.toObject();
            if (bind.contains(BINDTYPE_LINK)) {
                qDebug() << "  TimedMacro - Link:" << bind[BINDTYPE_LINK].toObject()[VALUE].toString();
                if (bind.contains(DELAY))
                    qDebug() << "   Delay:" << bind[DELAY].toInt();
            } else if (bind.contains(BINDTYPE_COMBO)) {
                QJsonArray keys = bind[BINDTYPE_COMBO].toArray();
                for (const QJsonValue& k : keys)
                    qDebug() << "  TimedMacro - Combo:" << k.toString();
            }
        }
    } else if (bind_key == BINDTYPE_REPEAT) {
        QJsonObject repeat = remapping[bind_key].toObject();
        qDebug() << "  Repeat Time Delay:" << repeat[TIME_DELAY].toInt();
        qDebug() << "  Repeat Count:" << repeat[TIMES_TO_EXECUTE].toInt();
        QJsonObject link = repeat[BINDTYPE_LINK].toObject();
        qDebug() << "   Repeat Link:" << link[VALUE].toString();

        if (repeat.contains(CANCEL_TRIGGER)) {
            QJsonObject cancel = repeat[CANCEL_TRIGGER].toObject();
            QJsonObject cancelLink = cancel[TRIGGERTYPE_LINK].toObject();
            qDebug() << "   Cancel Trigger:" << cancelLink[VALUE].toString();
        }
    } else if (bind_key == BINDTYPE_SWAPLAYER) {
        qDebug() << "  Swap to Layer:" << remapping[bind_key].toObject()[LAYER_NUM].toInt();
    } else if (bind_key == BINDTYPE_APPOPEN) {
        qDebug() << "  Open App:" << remapping[bind_key].toObject()[APP_NAME].toString();
    } else {
        qCritical() << "Unkown bind:" << bind_key;
    }
    return bind;
}

Trigger readTrigger(const QJsonObject& remapping, QString trigger_key) {
    Trigger trigger;
    if (trigger_key == TRIGGERTYPE_LINK) {
        QJsonObject timed = remapping[trigger_key].toObject();
        trigger = Trigger{
            .type = trigger_key,
            .vk = stringToKey(remapping[trigger_key].toObject()[VALUE].toString())
        };
    } else if (trigger_key == TRIGGERTYPE_TIMED) {
        QJsonArray keyTimePairs = remapping[KEY_TIME_PAIRS].toArray();
        QList<KeyTimePair> keyTimePairsa;
        for (const QJsonValue& pairVal : keyTimePairs) {
            QJsonObject pair = pairVal.toObject();
            qDebug() << "  Key:" << pair[VALUE].toString()
                     << " Delay:" << pair.value(DELAY).toInt();
            keyTimePairsa.append(KeyTimePair{
                .keyVk = stringToKey(pair[VALUE].toString()),
                .delay = pair[DELAY].toInt(0)
            });
        }
        TimedKeyBind sequence{
            .capture = remapping[CAPTURE].toBool(),
            .release = remapping[RELEASE].toBool(),
            .keyTimePairs = keyTimePairsa
        };
        trigger = Trigger{
            .type = trigger_key,
            .sequence = sequence
        };
    } else if (trigger_key == TRIGGERTYPE_HOLD) {
        QJsonObject hold = remapping[trigger_key].toObject();
        qDebug() << "  Hold Value:" << hold[VALUE].toString()
                 << " Wait:" << hold[WAIT].toInt();
    } else if (trigger_key == TRIGGERTYPE_APPFOCUSED) {
        qDebug() << "  App Focus:" << remapping[trigger_key].toObject()[APP_NAME].toString();
    } else {
        qCritical() << "Unkown trigger:" << trigger_key;
    }
    return trigger;
}

Trigger parseRemapping(const QJsonObject& remapping) {
    QString trigger_key;
    QString bind_key;
    for (const QString& key : remapping.keys()) {
        qDebug() << "Remapping Type:" << key;
        if (key.endsWith(TRIGGER)) {
            trigger_key = key;
        } else if (key.endsWith(BIND)) {
            bind_key = key;
        } else {
            qCritical() << "Key not Trigger or Bind" << key;
        }
    }

    Bind bind = readBind(remapping, bind_key);
    Trigger trigger = readTrigger(remapping, trigger_key);

    trigger.bind = bind;
    return trigger;
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
