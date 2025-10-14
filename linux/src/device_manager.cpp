#include "device_manager.h"
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QVector>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <qlogging.h>
#include <systemd/sd-device.h>
#include <unistd.h>

using std::cout;
using std::endl;

// TODO: Create a more robust config system for all OSs
const QString DEFAULT_CONFIG_PATH =
    QDir::homePath() + "/.config/clickr/config.json";

QString retrieve_eventX(QString config_file_path) {
    qDebug() << "Searching for eventX in path: " << config_file_path;
    QString eventX = "";
    QFile config(config_file_path);

    if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Could not open config file at path: " +
                           config_file_path;
    }

    QTextStream in(&config);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.contains("keyboard=")) {
            int equal_idx = line.indexOf("=");
            eventX = line.mid(equal_idx + 1);
            qDebug() << "Found eventX: " << eventX;
            break;
        }
    }

    config.close();
    return eventX;
}

QString retrieve_eventX() { return retrieve_eventX(DEFAULT_CONFIG_PATH); }

bool record_eventX(QString eventX_path, QString config_file_path) {
    bool recorded = false;
    bool no_prev_keyb = true;
    QStringList all_lines;

    QFile config(config_file_path);

    // Check if file exists; if not, create it with a "keyboard=" line
    if (!config.exists()) {
        QDir configDir = QFileInfo(config_file_path).dir();
        if (!configDir.exists()) {
            configDir.mkpath(".");
        }

        if (config.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&config);
            out << "keyboard=" << eventX_path << "\n";
            config.close();
            return true;
        } else {
            qCritical() << "Failed to create config file at path: " +
                               config_file_path;
            return false;
        }
    }

    // Read the current config
    if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Could not open config file at path: " +
                           config_file_path;
        return false;
    }

    QTextStream in(&config);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains("keyboard=")) {
            no_prev_keyb = false;
            line = "keyboard=" + eventX_path;
        }
        all_lines.append(line);
    }
    config.close();

    // Write the updated config
    if (!config.open(QIODevice::WriteOnly | QIODevice::Text |
                     QIODevice::Truncate)) {
        qCritical() << "Could not open config file for writing at path: " +
                           config_file_path;
        return false;
    }

    QTextStream out(&config);
    for (const QString &line : all_lines) {
        out << line << "\n";
    }
    if (no_prev_keyb) {
        out << "keyboard=" + eventX_path << "\n";
    }

    config.close();
    recorded = true;

    return recorded;
}

bool record_eventX(QString eventX_path) {
    return record_eventX(eventX_path, DEFAULT_CONFIG_PATH);
}

QString detect_keyboard() {
    sd_device_enumerator *enumerator = nullptr;
    sd_device *device = nullptr;

    // Create a new enumerator object that will walk through udev's device list
    if (sd_device_enumerator_new(&enumerator) < 0) {
        qWarning() << "Failed to create sd-device enumerator, falling back to "
                      "spacebar detection";
        return detect_keyboard_fallback();
    }

    // Restrict enumeration to the "input" subsystem (all input devices).
    // Pass `true` to match devices from this subsystem and its parents.
    sd_device_enumerator_add_match_subsystem(enumerator, "input", true);

    // Further restrict to devices udev has identified as keyboards.
    // Udev sets ID_INPUT_KEYBOARD=1 in its rules for actual keyboard devices.
    sd_device_enumerator_add_match_property(enumerator, "ID_INPUT_KEYBOARD",
                                            "1");

    // Collect device paths of all matching candidates
    QVector<QString> device_candidates;
    // Get the first matching device and iterate until none left
    sd_device *d = sd_device_enumerator_get_device_first(enumerator);
    for (; d != nullptr; d = sd_device_enumerator_get_device_next(enumerator)) {
        const char *devnode = nullptr;

        // Preferred: use the kernel-provided device node (e.g.
        // "/dev/input/event3")
        if (sd_device_get_devname(d, &devnode) >= 0 && devnode) {
            // copy devnode into a QString while enumerator/device is still
            // valid
            device_candidates.push_back(QString::fromUtf8(devnode));
        } else {
            // Fallback: if no devnode, try sysname (e.g. "event3") and
            // construct path
            const char *sysname = nullptr;
            if (sd_device_get_sysname(d, &sysname) >= 0 && sysname) {
                QString path = "/dev/input/" + QString::fromUtf8(sysname);
                device_candidates.push_back(path);
            }
        }
    }

    // Free the enumerator object now that we've collected candidates
    sd_device_enumerator_unref(enumerator);

    qDebug() << "Found" << device_candidates.size()
             << "keyboard candidates via sd-device.";

    QList<QString> keyboard_candidates;
    // If only one candidate was found, assume it is the keyboard
    if (device_candidates.size() == 1) {
        return device_candidates[0];
    }
    // If no devices were found, fall back to interactive detection
    else if (device_candidates.size() == 0) {
        qDebug()
            << "No keyboard devices found, falling back to spacebar detection.";
        return detect_keyboard_fallback();
    } else {
        // We have multiple candidates, refine using libevdev heuristics
        // to confirm which device looks like a "full" keyboard
        for (const QString &path : device_candidates) {
            int fd = open(path.toUtf8().constData(), O_RDONLY | O_NONBLOCK);
            if (fd < 0) {
                qWarning() << "Failed to open" << path;
                continue;
            }

            struct libevdev *dev = nullptr;
            // Initialize a libevdev device object from the fd
            if (libevdev_new_from_fd(fd, &dev) < 0) {
                close(fd);
                continue;
            }

            // Run heuristic: device must support typical alphanumeric +
            // modifier keys
            if (is_full_keyboard(dev)) {
                qDebug() << "Possible keyboard candidate at: " << path;
                keyboard_candidates.push_back(path);
            }

            // Clean up
            libevdev_free(dev);
            close(fd);
        }
    }

    // If multiple devices matched heuristics, fall back to interactive
    // detection
    if (keyboard_candidates.size() > 1) {
        qDebug() << "Multiple possible keyboards detected, falling back to "
                    "spacebar detection";
        return detect_keyboard_fallback();
    }
    // If no device matched heuristics, fall back to interactive detection
    else if (keyboard_candidates.size() == 0) {
        qDebug() << "No keyboard matched heuristics, falling back to spacebar "
                    "detection.";
        return detect_keyboard_fallback();
    } else {
        // Only one possible keyboard was found, return it
        qDebug() << "Only one possible keyboard candidate detected at path: "
                 << keyboard_candidates[0];
        return keyboard_candidates[0];
    }
}

bool is_full_keyboard(libevdev *dev) {
    if (!dev)
        return false;

    bool looks_like_keyboard =
        libevdev_has_event_type(dev, EV_KEY) &&
        libevdev_has_event_code(dev, EV_KEY, KEY_A) &&
        libevdev_has_event_code(dev, EV_KEY, KEY_Z) &&
        libevdev_has_event_code(dev, EV_KEY, KEY_SPACE) &&
        libevdev_has_event_code(dev, EV_KEY, KEY_ENTER) &&
        libevdev_has_event_code(dev, EV_KEY, KEY_LEFTSHIFT);

    return looks_like_keyboard;
}

QString detect_keyboard_fallback() {
    uint timeout_seconds = 30;
    QString keyb_path;
    // Open the /dev/input/ directory
    DIR *dir = opendir("/dev/input/");
    if (!dir) {
        qCritical() << "Failed to open /dev/input/ directory";
        return keyb_path;
    }

    struct dirent *entry;
    QVector<QString> possible_keyb_paths;
    QVector<int> fds;
    QVector<libevdev *> devices;

    int event_counter = 0;
    int possible_keyboards = 0;

    // Loop over each entry in /dev/input/
    while ((entry = readdir(dir)) != nullptr) {
        // Only process "eventX" files
        if (strncmp(entry->d_name, "event", 5) == 0) {
            QString event_path = "/dev/input/" + QString(entry->d_name);
            event_counter++;

            struct libevdev *dev = nullptr;

            int fd =
                open(event_path.toUtf8().constData(), O_RDONLY | O_NONBLOCK);
            if (fd < 0) {
                qWarning() << "Failed to open " << event_path;
                continue;
            }

            // Initialize the evdev device
            if (libevdev_new_from_fd(fd, &dev) < 0) {
                qWarning() << "Failed to initialize evdev device";
                continue;
            }

            // If a keyboard device has spacebar and the x key, it is most
            // likely an actual keyboard
            bool has_key_events = is_full_keyboard(dev);
            if (has_key_events) {
                possible_keyb_paths.push_back(event_path);
                cout << "possible keyb path: " + event_path.toStdString()
                     << endl; // Still need to verify that this is actually a
                              // keyboard
                possible_keyboards++;
                fds.push_back(fd);
                devices.push_back(dev);
            } else {
                // Clean up
                libevdev_free(dev);
                close(fd);
            }
        }
    }
    cout << "Number of /dev/input/eventX devices: " << event_counter << endl;
    cout << "Number of possible keyboards: " << possible_keyboards << endl;

    // Now we listen to key events to determine which device is indeed the
    // keyboard
    cout << "Press SPACEBAR to identify the correct keyboard device. This will "
            "time out after "
         << timeout_seconds << " seconds." << endl;
    bool found_keyb = false;
    QElapsedTimer timer;
    timer.start();

    while (!found_keyb) {
        for (size_t i = 0; i < devices.size(); i++) {
            struct input_event event;
            while (libevdev_next_event(devices[i], LIBEVDEV_READ_FLAG_NORMAL,
                                       &event) == 0) {
                if (event.type == EV_KEY && event.code == KEY_SPACE &&
                    event.value == 1) // Space has been pressed on devices[i]
                {
                    cout << "Detected spacebar press on: "
                         << possible_keyb_paths[i].toStdString() << endl;
                    keyb_path = possible_keyb_paths[i];
                    found_keyb = true;
                    break;
                }
            }
        }

        if (found_keyb) {
            break;
        }

        if (timer.elapsed() >= timeout_seconds * 1000) {
            qDebug() << timeout_seconds
                     << " seconds have passed and a keyboard has not been "
                        "detected. Exiting detection loop.";
            break;
        }
    }

    record_eventX(keyb_path);

    // Clean up all devices
    for (size_t i = 0; i < devices.size(); i++) {
        libevdev_free(devices[i]);
        close(fds[i]);
    }
    devices.clear();
    fds.clear();

    return keyb_path;
}

int setup_uinput_device() {
    int uinp_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinp_fd < 0) {
        qCritical() << "Failed to open /dev/uinput";
        return -1;
    }

    // Declare which event types the keyboard supports.
    // EV_KEY -> This device can generate key events (keyboard keys, mouse,
    // buttons).
    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    // EV_SYN -> Synchronization events, used to mark the end of a batch of
    // events.
    ioctl(uinp_fd, UI_SET_EVBIT, EV_SYN);

    // Register all possible keys as available to be remapped
    for (int i = 0; i <= KEY_MAX; i++) {
        ioctl(uinp_fd, UI_SET_KEYBIT, i);
    }

    // Create uinput device
    struct uinput_setup uinput_device;
    memset(&uinput_device, 0, sizeof(uinput_device));
    uinput_device.id.bustype = BUS_USB; // Pretend to be a USB device
    uinput_device.id.vendor = 0x1;      // Arbitrary
    uinput_device.id.product = 0x1;     // Arbitrary
    // Device name that will show up in /proc/bus/input/devices or evtest
    strncpy(uinput_device.name, "clickr_virtual_keyboard\0",
            sizeof(uinput_device.name));

    if (ioctl(uinp_fd, UI_DEV_SETUP, &uinput_device) <
            0 || // Configure the device with the provided info
        ioctl(uinp_fd, UI_DEV_CREATE) <
            0) { // Tells the kernel to create the device
        qCritical() << "Failed to create uinput device";
        close(uinp_fd);
        return -1;
    }

    // Return the device's file descriptor
    return uinp_fd;
}
