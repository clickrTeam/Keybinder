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
QMap<int, int> timedKeyProgress; // represents progress towards finish the timed macro.
int next_key; // the next expected key in a timed macro. Never includes the first.
int first_key; // the first key in a expected macro.

bool thenRelease; // true if some key is captured to release.
int capture_and_release_key; // the key captured to release.


bool mapKeyDownToBind(int virtualKey) {
    qDebug() << virtualKey;

    // Timed
    bool isAFirstKey = activeLayer.timedKeyBinds.contains(virtualKey);
    if (isAFirstKey || next_key == virtualKey) {
        qDebug() << "Timed";
        if (next_key != virtualKey && isAFirstKey && first_key != virtualKey) {
            // Starting a diffrent timed since firstKey is diffrent, and is not nextkey.
            captureAndRelease(); // starting a diffrent timed
            first_key = virtualKey; // this lets us use timed keys with other keys
        }
        TimedKeyBind kybnd = activeLayer.timedKeyBinds[first_key];

        // Progress
        if (timedKeyProgress.contains(first_key)) {
            qDebug() << "Incrementing Progress";
            timedKeyProgress[first_key]++;
        } else {
            qDebug() << "Starting Progress";
            timedKeyProgress[first_key] = 1;
        }
        if (timedKeyProgress[first_key] == 1 && thenRelease) { // If starting a diffrent timed
            qDebug() << "Stopping CnR";
            thenRelease = false; // stop capture and release
            press(capture_and_release_key);
        }

        // Finished proggress or setup next
        if (timedKeyProgress[first_key] == kybnd.keyTimePairs.count()) {
            qDebug() << "Finished Progress";
            thenRelease = false;
            // last keybind activate
            press(kybnd.bind);
            timedKeyProgress[first_key] = 0;
            return true;
        } else {
            qDebug() << "Setting next key" << kybnd.keyTimePairs[timedKeyProgress[first_key]].keyVk;
            next_key = kybnd.keyTimePairs[timedKeyProgress[first_key]].keyVk;
        }

        // Capture and Release logic
        if (kybnd.capture && kybnd.release) {
            qDebug() << "Capture & Realeasing key";
            capture_and_release_key = virtualKey;
            thenRelease = true;
            int ms = kybnd.keyTimePairs[timedKeyProgress[first_key]].time;
            QTimer::singleShot(ms, []() {
                captureAndRelease();
            });
            return true;
        } else if (kybnd.capture) {
            qDebug() << "Capturing key";
            return true;
        } else if (kybnd.release) {
            qDebug() << "Releasing key";
            return false;
        } else
            qCritical() << "FELL THROUGH CAPTURE AND RELEASE!" << kybnd.capture << kybnd.release;
    } else
        captureAndRelease(); // Cancel capture and release if bind is broken.

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


void captureAndRelease() {
    timedKeyProgress[first_key] = 0; // reset progress
    if (thenRelease) {
        thenRelease = false; // stop capture and release
        press(capture_and_release_key); // release captured key
        qDebug() << "Stopping CnR";
    }
}
