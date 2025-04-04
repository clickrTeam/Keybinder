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
int capture_and_release_key;

bool mapKeyDownToBind(int virtualKey) {
    qDebug() << virtualKey;

    // Timed
    if (activeLayer.timedKeyBinds.contains(virtualKey) || next_key == virtualKey) {
        qDebug() << "Timed";
        if (activeLayer.timedKeyBinds.contains(virtualKey) && first_key != virtualKey) // this goes second, to ensure a key can be mapped to mapped key
            first_key = virtualKey;
        TimedKeyBind kybnd = activeLayer.timedKeyBinds[first_key];

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


        if (timedKeyProgress[first_key] == kybnd.keyTimePairs.count()) {
            qDebug() << "Finished Progress";
            thenRelease = false;
            // last keybind activate
            press(kybnd.bind);
            timedKeyProgress[first_key] = 0;
            return true;
        } else {
            qDebug() << "Setting next key" << kybnd.keyTimePairs[timedKeyProgress[first_key]].keyValue;
            next_key = kybnd.keyTimePairs[timedKeyProgress[first_key]].keyValue;
        }

        if (kybnd.capture && kybnd.release) {
            qDebug() << "Capture & Realeasing key";
            capture_and_release_key = virtualKey;
            thenRelease = true;
            QTimer::singleShot(1000, []() {
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
        captureAndRelease();
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
    timedKeyProgress[first_key] = 0;
    if (thenRelease) {
        qDebug() << "Stopping CnR";
        thenRelease = false; // stop capture and release
        press(capture_and_release_key);
    }
}
