#include "mapper.h"
#include "daemon.h"
#include "event.h"
#include <QTimer>

Mapper::Mapper(Profile &profile) : profile(profile) { this->cur_layer = 0; }
Mapper::~Mapper() {}

void Mapper::set_daemon(Daemon *d) { daemon = d; }

bool Mapper::mapInput(InputEvent e) {
    Layer &activeLayer = this->profile.layers[this->cur_layer];

    if (e.type == KeyEventType::Relase) {
        // TODO
        return false;
    }
    int virtualKey = e.keycode;

    qDebug() << virtualKey;

    // Timed
    bool isAFirstKey = activeLayer.timedKeyBinds.contains(virtualKey);
    if (isAFirstKey || next_key == virtualKey) {
        qDebug() << "Timed";
        if (next_key != virtualKey && isAFirstKey && first_key != virtualKey) {
            // Starting a diffrent timed since firstKey is diffrent, and is not
            // nextkey.
            captureAndRelease(); // starting a diffrent timed
            first_key =
                virtualKey; // this lets us use timed keys with other keys
        }
        Trigger trigger = activeLayer.timedKeyBinds[first_key];
        TimedKeyBind kybnd = *trigger.sequence;

        // Progress
        if (timedKeyProgress.contains(first_key)) {
            qDebug() << "Incrementing Progress";
            timedKeyProgress[first_key]++;
        } else {
            qDebug() << "Starting Progress";
            timedKeyProgress[first_key] = 1;
        }
        if (timedKeyProgress[first_key] == 1 &&
            thenRelease) { // If starting a diffrent timed
            qDebug() << "Stopping CnR";
            thenRelease = false; // stop capture and release
            daemon->send_key(capture_and_release_key);
        }

        // Finished proggress or setup next
        if (timedKeyProgress[first_key] == kybnd.keyTimePairs.count()) {
            qDebug() << "Finished Progress";
            thenRelease = false;
            // last keybind activate
            activateBind(trigger.bind);
            timedKeyProgress[first_key] = 0;
            return true;
        } else {
            qDebug() << "Setting next key"
                     << kybnd.keyTimePairs[timedKeyProgress[first_key]].keyVk;
            next_key = kybnd.keyTimePairs[timedKeyProgress[first_key]].keyVk;
        }

        // Capture and Release logic
        if (kybnd.capture && kybnd.release) {
            qDebug() << "Capture & Realeasing key";
            capture_and_release_key = virtualKey;
            thenRelease = true;
            // ms is the delay on the current key pressed.
            int ms = kybnd.keyTimePairs[timedKeyProgress[first_key] - 1].delay;
            QTimer::singleShot(ms, [&]() { this->captureAndRelease(); });
            return true;
        } else if (kybnd.capture) {
            qDebug() << "Capturing key";
            return true;
        } else if (kybnd.release) {
            qDebug() << "Releasing key";
            return false;
        } else
            qCritical() << "FELL THROUGH CAPTURE AND RELEASE!" << kybnd.capture
                        << kybnd.release;
    } else
        captureAndRelease(); // Cancel capture and release if bind is broken.

    // TAP
    if (activeLayer.tapKeyBinds.contains(virtualKey)) {
        qDebug() << "Tap";
        daemon->send_key(activeLayer.tapKeyBinds[virtualKey]);
        return true;
    }
    return false;
}

void Mapper::activateBind(Bind bind) {
    switch (bind.type) {
    case B_LINK:
        daemon->send_key(*bind.vk);
        break;

    case COMBO:
        daemon->send_keys(*bind.combo);
        break;

    case MACRO:
        for (const Bind& other_bind : *bind.macro) {
            activateBind(other_bind);
        }
        break;

    case TIMEDMACRO:
        // Handle Timed Macro Bind
        break;

    case REPEAT:
        // Handle Repeat Bind
        break;

    case SWAPLAYER:
        // Handle Swap Layer Bind
        break;

    case APPOPEN:
        // Handle App Open Bind
        break;

    default:
        qCritical() << "Unkown bind type in mapper:" << bind.type;
        // Optional: handle unknown type
        break;
    }
}


void Mapper::captureAndRelease() {
    timedKeyProgress[first_key] = 0; // reset progress
    if (thenRelease) {
        thenRelease = false;                       // stop capture and release
        daemon->send_key(capture_and_release_key); // release captured key
        qDebug() << "Stopping CnR";
    }
}
