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
        Keybind kybnd;
        QJsonObject keybind = keybindValue.toObject();
        QJsonObject key = keybind.value(KEYBIND_KEY).toObject();
        kybnd.key = readKey(key);
        QJsonObject bind = keybind.value(KEYBIND_BIND).toObject();
        kybnd.bind = readBind(bind);
        lyr.keybinds.append(kybnd);
    }
    for (const Keybind &keybind: std::as_const(lyr.keybinds)) {
        if (keybind.key.type == "tap")
            lyr.tapKeyBinds[keybind.key.value] = keybind.bind.value;
    }
    return lyr;
}

Key readKey(QJsonObject key) {
    QString keyType = key.value(TYPE).toString();
    QString keyValue = key.value(VALUE).toString();
    Key ky;
    ky.type = keyType;
    ky.value = keyValue;

    // Output key information
    qDebug() << "Key Type:" << keyType;
    qDebug() << "Key Value:" << keyValue;
    return ky;
}

Bind readBind(QJsonObject bind) {
    QString bindType = bind.value(TYPE).toString();
    QString bindValue = bind.value(VALUE).toString();
    Bind bnd;
    bnd.type = bindType;
    bnd.value = bindValue;

    // Output bind information
    qDebug() << "Bind Type:" << bindType;
    qDebug() << "Bind Value:" << bindValue;
    return bnd;
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
