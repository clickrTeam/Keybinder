#ifndef PROFILE_H
#define PROFILE_H

#include <QList>
#include <QMap>
#include <QString>
using std::optional;

enum class KeyCode {
    A = 0,
    B = 1,
    C = 2,
    D = 3,
    E = 4,
    // TOOD
};

// Short enums for internal use (Bind Types)
enum BT {
    B_LINK,
    COMBO,
    MACRO,
    TIMEDMACRO,
    REPEAT,
    SWAPLAYER,
    APPOPEN,
    B_UNKOWN
};

// Short enums for internal use (Trigger Types)
enum TT {
    T_LINK,
    TIMED,
    HOLD,
    APPFOCUSED,
    T_UNKOWN
};

// Class to represent the Profile structure
struct KeyTimePair {
    int keyVk;   // The key value (e.g., "w")
    int delay;   // The associated time in ms
};
//TODO repeat bind
struct Bind {
    BT type;  // The bind type (e.g., "tap")
    optional<int> vk;
    optional<QList<int>> combo;
    optional<QList<Bind>> macro;
    optional<QList<QPair<Bind, int>>> timedMacro;
    optional<QString> app_name;
    optional<int> layer_index;
};
struct TimedKeyBind {
    bool capture;
    bool release;
    QList<KeyTimePair> keyTimePairs; // Vector to store key-time pairs
};
struct Trigger {
  public:
    Bind bind;
    TT type;  // The key type (e.g., "tap")
    optional<int> vk;
    optional<TimedKeyBind> sequence;
    optional<QString> app_name;
};
struct Layer {
  public:
    QString layerName;       // The layer name (e.g., "Gaming Layer")
    QList<Trigger> keybinds; // List of keybinds in this layer
    QMap<int, Trigger> associatedKeys;
    QMap<int, int> tapKeyBinds;
    QMap<int, Trigger> timedKeyBinds; // First int is the first key in the array
};
struct Profile {
  public:
    QString name;        // The name of the profile (e.g., "Default Profile")
    QList<Layer> layers; // List of layers in the profile
    bool isNull = true;
};

#endif // PROFILE_H
