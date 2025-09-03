#include "linux_configure.h"
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QVector>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <unistd.h>

using std::cout;
using std::endl;

// TODO: Figure out where we want to store the config file by default
const QString DEFAULT_CONFIG_PATH =
    QDir::homePath() + "/.config/clickr/config.json";

QString retrieve_eventX(QString config_file_path) {
    QString eventX = "";
    QFile config(config_file_path);

    if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Could not open config file at path: " + config_file_path
                    << Qt::endl;
    }

    QTextStream in(&config);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.contains("keyboard=")) {
            int equal_idx = line.indexOf("=");
            eventX = line.mid(equal_idx + 1);
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
                               config_file_path
                        << Qt::endl;
            return false;
        }
    }

    // Read the current config
    if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Could not open config file at path: " + config_file_path
                    << Qt::endl;
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
                           config_file_path
                    << Qt::endl;
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
    uint timeout_seconds = 30;
    QString keyb_path;
    // Open the /dev/input/ directory
    DIR *dir = opendir("/dev/input/");
    if (!dir) {
        qCritical() << "Failed to open /dev/input/ directory" << Qt::endl;
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
                qCritical() << "Failed to open " << event_path << Qt::endl;
                return keyb_path;
            }

            // Initialize the evdev device
            if (libevdev_new_from_fd(fd, &dev) < 0) {
                qCritical() << "Failed to initialize evdev device" << Qt::endl;
                close(fd);
                return keyb_path;
            }

            // If a keyboard device has spacebar and the x key, it is most
            // likely an actual keyboard
            bool has_key_events =
                libevdev_has_event_type(dev, EV_KEY) &&
                libevdev_has_event_code(dev, EV_KEY, KEY_SPACE) &&
                libevdev_has_event_code(dev, EV_KEY, KEY_X);
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
                        "detected. Exiting detection loop."
                     << Qt::endl;
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
