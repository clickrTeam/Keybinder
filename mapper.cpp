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

// Timed varibles
QMap<int, int> timedKeyProgress;
int next_key;
int first_key;
bool thenRelease;
int capture_and_release_bind;

bool mapKeyDownToBind(int virtualKey) {
    qDebug() << virtualKey;

    // Timed
    if (activeLayer.timedKeyBinds.contains(virtualKey) || next_key == virtualKey) {
        if (next_key == virtualKey)
            virtualKey = first_key;
        else if (activeLayer.timedKeyBinds.contains(virtualKey)) // this goes second, to ensure a key can be mapped to mapped key
            first_key = virtualKey;
        TimedKeyBind kybnd = activeLayer.timedKeyBinds[virtualKey];

        if (timedKeyProgress.contains(virtualKey)) {
            timedKeyProgress[virtualKey]++;
        } else {
            timedKeyProgress[virtualKey] = 1;
        }
        if (timedKeyProgress[virtualKey] == kybnd.keyTimePairs.count()) {
            if (thenRelease) {
                thenRelease = false;
                press(capture_and_release_bind);
            }
            // last keybind activate
            press(kybnd.bind);
            timedKeyProgress[virtualKey] = 0;
            return true;
        } else {
            next_key = kybnd.keyTimePairs[timedKeyProgress[virtualKey]].keyValue;
        }

        if (kybnd.capture && kybnd.release) {
            capture_and_release_bind = kybnd.bind;
            int bind = capture_and_release_bind;
            thenRelease = true;
            QTimer::singleShot(100, [bind]() {
                captureAndRelease(bind);
            });
        } else if (kybnd.capture) {
            return true;
        } else if (kybnd.release) {
            return false;
        }
    } else if (thenRelease) {
        thenRelease = false; // stop capture and release
        press(capture_and_release_bind);
    }
    // TAP
    if (activeLayer.tapKeyBinds.contains(virtualKey)) {
        qDebug() << "Tap";
        press(activeLayer.tapKeyBinds[virtualKey]);
        return true;
    }
    return false;
}

bool mapKeyUpToBind(int virtualKey) {
    return false;
    // TODO
}

void captureAndRelease(int bind) {
    if (!thenRelease)
        return;
    press(bind);
}
