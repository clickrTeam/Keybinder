#include <QHash>
#include <QString>
#include "keymap.h"


void KeyMap::insert(const QString& name, int keyCode) {
    strToKey[name] = keyCode;
    keyToStr[keyCode] = name;
}

int KeyMap::getKey(const QString& name) const {
    return strToKey.value(name, -1);
}

QString KeyMap::getName(int keyCode) const {
    return keyToStr.value(keyCode, QString());
}

bool KeyMap::containsName(const QString& name) const {
    return strToKey.contains(name);
}

bool KeyMap::containsKey(int keyCode) const {
    return keyToStr.contains(keyCode);
}
