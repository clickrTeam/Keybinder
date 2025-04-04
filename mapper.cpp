#include "mapper.h"
#include <QTimer>
#ifdef _WIN32
#include "win/deamon.h"
#elif defined(__APPLE__)
// TODO change
#elif defined(__linux__)
#include "ubuntu/learning/SimpleRemap.cpp"
#else
#error "Unknown operating system"
#endif

Profile activeProfile;
Layer activeLayer;

void setProfile(Profile loaded) {
    activeProfile = loaded;
    activeLayer = activeProfile.layers[0];
    qDebug() << activeLayer.tapKeyBinds.keys();
}

QMap<int, int> timedKeyProgress;
int next_key;
int first_key;

bool mapKeyDownToBind(int virtualKey) {
    qDebug() << virtualKey;

    if (activeLayer.tapKeyBinds.contains(virtualKey)) {
        qDebug() << "Tap";
        press(activeLayer.tapKeyBinds[virtualKey]);
        return true;
    }
    else if (activeLayer.timedKeyBinds.contains(virtualKey) || next_key == virtualKey) {
        if (activeLayer.timedKeyBinds.contains(virtualKey))
            first_key = virtualKey;
        else if (next_key == virtualKey) {
            virtualKey = first_key;
        }
        TimedKeyBind kybnd = activeLayer.timedKeyBinds[virtualKey];

        if (timedKeyProgress.contains(virtualKey)) {
            timedKeyProgress[virtualKey]++;
        } else {
            timedKeyProgress[virtualKey] = 1;
        }
        if (timedKeyProgress[virtualKey] == kybnd.keyTimePairs.count()) {
            // last keybind activate
            press(kybnd.bind);
            timedKeyProgress[virtualKey] = 0;
            return true;
        } else {
            next_key = kybnd.keyTimePairs[timedKeyProgress[virtualKey]].keyValue;
        }

        if (kybnd.capture && kybnd.release) {
            QTimer::singleShot(100, [arg1, arg2]() {
                delayedFunction(arg1, arg2);
            });
        } else if (kybnd.capture) {
            return true;
        } else if (kybnd.release) {
            return false;
        }
    }
    return false;
}

bool mapKeyUpToBind(int virtualKey) {
    return false;
    // TODO
}
