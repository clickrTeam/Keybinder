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

// TT stringToTT(const QString& type) {
//     if (type == TRIGGERTYPE_LINK) return T_LINK;
//     if (type == TRIGGERTYPE_TIMED) return TIMED;
//     if (type == TRIGGERTYPE_HOLD) return HOLD;
//     if (type == TRIGGERTYPE_APPFOCUSED) return APPFOCUSED;
//     return T_UNKOWN;
// }
// BT stringToBT(const QString& type) {
//     if (type == BINDTYPE_LINK) return B_LINK;
//     if (type == BINDTYPE_COMBO) return COMBO;
//     if (type == BINDTYPE_MACRO) return MACRO;
//     if (type == BINDTYPE_TIMEDMACRO) return TIMEDMACRO;
//     if (type == BINDTYPE_REPEAT) return REPEAT;
//     if (type == BINDTYPE_SWAPLAYER) return SWAPLAYER;
//     if (type == BINDTYPE_APPOPEN) return APPOPEN;
//     return B_UNKOWN;
// }

Layer readLayer(QJsonObject layer) {
    Layer lyr;
    qDebug() << "Layer:" << layer[LAYER_NAME].toString();

    QJsonArray remappings = layer[LAYER_KEYBINDS].toArray();
    for (const QJsonValue& remapVal : remappings) {
        lyr.keybinds.append(parseRemapping(remapVal.toObject()));
    }
    qDebug() << "triggers Found" << lyr.keybinds.count() << "triggers in json" << remappings.count();
    for (const Trigger& trigger : lyr.keybinds) {
        qDebug() << trigger.type;
        if (trigger.type == T_LINK) {
            lyr.tapKeyBinds[*trigger.vk] = *trigger.bind.vk;
        } else if (trigger.type == TIMED) {
            TimedKeyBind tkb = *trigger.sequence;
            qCritical() << "No keyTimePairs in trigger" << tkb.keyTimePairs.count();
            lyr.timedKeyBinds[tkb.keyTimePairs[0].keyVk] = trigger;
        } else {
            qDebug() << "Not setup" << trigger.type;
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
            .type = B_LINK,
            .vk = stringToKey(remapping[bind_key].toObject()[VALUE].toString())
        };
    } else if (bind_key == BINDTYPE_COMBO) {
        QJsonArray combo = remapping[bind_key].toArray();
        QList<int> keys;
        for (const QJsonValue& val : combo) {
            qDebug() << "  Combo Key:" << val.toString();
            keys.append(stringToKey(val.toString()));
        }
        bind = Bind{
            .type = COMBO,
            .combo = keys
        };
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
        bind = Bind{
            .type = MACRO,
        };
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
        bind = Bind{
            .type = TIMEDMACRO,
        };
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
        bind = Bind{
            .type = REPEAT,
        };
    } else if (bind_key == BINDTYPE_SWAPLAYER) {
        qDebug() << "  Swap to Layer:" << remapping[bind_key].toObject()[LAYER_NUM].toInt();
        bind = Bind{
            .type = SWAPLAYER,
        };
    } else if (bind_key == BINDTYPE_APPOPEN) {
        qDebug() << "  Open App:" << remapping[bind_key].toObject()[APP_NAME].toString();
        bind = Bind{
            .type = APPOPEN,
        };
    } else {
        qCritical() << "Unkown bind:" << bind_key;
        bind = Bind{
            .type = B_UNKOWN,
        };
    }
    return bind;
}

Trigger readTrigger(const QJsonObject& remapping, QString trigger_key) {
    Trigger trigger;
    if (trigger_key == TRIGGERTYPE_LINK) {
        trigger = Trigger{
            .type = T_LINK,
            .vk = stringToKey(remapping[trigger_key].toObject()[VALUE].toString())
        };
    } else if (trigger_key == TRIGGERTYPE_TIMED) {
        QJsonObject timed = remapping[trigger_key].toObject();
        QJsonArray keyTimePairs = timed[KEY_TIME_PAIRS].toArray();
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
            .capture = timed[CAPTURE].toBool(),
            .release = timed[RELEASE].toBool(),
            .keyTimePairs = keyTimePairsa
        };
        trigger = Trigger{
            .type = TIMED,
            .sequence = sequence
        };
    } else if (trigger_key == TRIGGERTYPE_HOLD) {
        QJsonObject hold = remapping[trigger_key].toObject();
        qDebug() << "  Hold Value:" << hold[VALUE].toString()
                 << " Wait:" << hold[WAIT].toInt();
        trigger = Trigger{
            .type = HOLD,
        };
    } else if (trigger_key == TRIGGERTYPE_APPFOCUSED) {
        qDebug() << "  App Focus:" << remapping[trigger_key].toObject()[APP_NAME].toString();
        trigger = Trigger{
            .type = APPFOCUSED,
        };
    } else {
        qCritical() << "Unkown trigger:" << trigger_key;
        trigger = Trigger{
            .type = T_UNKOWN,
        };
    }
    return trigger;
}

Trigger parseRemapping(const QJsonObject& remapping) {
    QString trigger_key;
    QString bind_key;
    for (const QString& key : remapping.keys()) {
        // qDebug() << "Remapping Type:" << key;
        if (key.endsWith(TRIGGER)) {
            trigger_key = key;
        } else if (key.endsWith(BIND)) {
            bind_key = key;
        } else {
            qCritical() << "Key not Trigger or Bind" << key;
        }
    }
    qDebug() << trigger_key << " --> " << bind_key;

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
