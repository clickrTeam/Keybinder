#ifndef PROFILE_H
#define PROFILE_H

#include <QString>
#include <QMap>
#include <QList>

// Class to represent the Profile structure
class Key {
public:
    QString value;  // The key value (e.g., "w")
    QString type;   // The key type (e.g., "tap")
};
class Bind {
public:
    QString value;  // The bind value (e.g., "q")
    QString type;   // The bind type (e.g., "tap")
};
class Keybind {
public:
    Key key;    // Key object
    Bind bind;  // Bind object
};
class Layer {
public:
    QString layerName;  // The layer name (e.g., "Gaming Layer")
    QList<Keybind> keybinds;  // List of keybinds in this layer
    QMap<int, int> tapKeyBinds;
};
class Profile {
public:
    QString name;  // The name of the profile (e.g., "Default Profile")
    QList<Layer> layers;  // List of layers in the profile
    bool isNull = true;
};

#endif // PROFILE_H
