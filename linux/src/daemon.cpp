#include "daemon.h"
#include "device_manager.h"
#include "key_channel.h"
#include "key_map.h"
#include <QThread>
#include <stdexcept>

Daemon::Daemon(KeySender key_sender) : key_sender(key_sender) {
    // TODO: Possibly update with config file path
    event_keyb_path = retrieve_eventX();

    // No path keyb path was detected in config file
    if (event_keyb_path == "") {
        event_keyb_path = detect_keyboard();
        record_eventX(event_keyb_path);
    }
    qDebug() << "Daemon created";
    // Do stuff
}

Daemon::~Daemon() {
    cleanup();
    qDebug() << "Daemon destroyed" << Qt::endl;
}

void Daemon::cleanup() {
    if (this->is_running) {
        ioctl(uinput_fd, UI_DEV_DESTROY);
        close(uinput_fd);
        if (keyb != nullptr && keyb_fd >= 0) {
            libevdev_grab(
                keyb,
                LIBEVDEV_UNGRAB); // Release control of the physical keyboard
            libevdev_free(keyb);
            close(keyb_fd);

            // Explicitly set keyb and keyb_fd
            keyb_fd = -1;
            keyb = nullptr;
        }
        qDebug() << "Daemon cleaned up";
        is_running = false;
    } else {
        qDebug()
            << "cleanup() called but daemon not running. Exiting function.";
    }
}

void Daemon::start() {
    qDebug() << "Daemon started";

    // Open the identified keyboard device in read-write mode
    this->keyb_fd =
        open(event_keyb_path.toUtf8().constData(), O_RDWR | O_NONBLOCK);
    if (keyb_fd < 0) {
        qCritical() << "Failed to open device: " << strerror(errno);
        return;
    }

    if (libevdev_new_from_fd(this->keyb_fd, &keyb) < 0) {
        qCritical() << "Failed to initialize libevdev";
        return;
    }

    // Release all keys before grab to prevent key spam
    for (int i = 0; i <= KEY_MAX; i++) {
        send_key(i, 0, keyb_fd);
    }

    // Prevent the physical keyboard key presses from being typed
    if (libevdev_grab(keyb, LIBEVDEV_GRAB) < 0) {
        qCritical() << "Failed to grab the keyboard device";
        return;
    }

    // Set up uinput for key injection
    uinput_fd = setup_uinput_device();
    if (uinput_fd < 0) {
        qCritical() << "Failed to set up uinput";
        return;
    }

    bool termination_condition = false;
    QList<OutputEvent> event_list;
    is_running = true;
    while (!termination_condition) {
        // Check if the thread has been requested to be interrupted, if so exit
        // the loop. Otherwise this loop will block and prevent proper
        // termination
        termination_condition =
            QThread::currentThread()->isInterruptionRequested();
        struct input_event event;
        while (libevdev_next_event(keyb, LIBEVDEV_READ_FLAG_NORMAL, &event) ==
               0) {
            if (event.type == EV_KEY) {
                KeyEvent e;
                // TODO remove try catch
                try {
                    e.keycode = int_to_keycode.find_forward(event.code);
                    e.type = (event.value == 1) ? KeyEventType::Press
                                                : KeyEventType::Release;

                    if (key_sender.send_key(e)) {
                        // Suppressed by the mapper (i.e. replaced/mapped to
                        // something else)
                        continue;
                    } else {
                        // Inject original key if not mapped
                        event_list.append(e);
                        send_outputs(event_list);
                        event_list.clear();
                    }
                } catch (std::out_of_range) {
                    send_key(event.code, event.value, uinput_fd);
                }
            }
        }
    }
}

void Daemon::send_outputs(const QList<OutputEvent> &outputs) {
    foreach (OutputEvent event, outputs) {
        if (const KeyEvent *input = std::get_if<KeyEvent>(&event)) {

            bool type = input->type == KeyEventType::Press ? 1 : 0;
            int key_code = int_to_keycode.find_backward(input->keycode);
            send_key(key_code, type, uinput_fd);
            //
        } else {
            // TODO: The macos version should be compatable with this
            qWarning() << "RunScript is not implemented on Windows yet";
        }
    }
}

// state 1 for pressed, 0 for released
void Daemon::send_key(int key_code, int state, int fd) {
    struct input_event event;
    memset(&event, 0, sizeof(event));

    // Key press event
    event.type = EV_KEY;
    event.code = key_code;            // Key that we are sending
    event.value = state;              // Key up or down
    write(fd, &event, sizeof(event)); // Send the event

    // Synchronization event
    event.type = EV_SYN;
    // SYN_REPORT -> Used to synchronize and separate events into packets
    //               of input data occurring at the same moment in time.
    event.code = SYN_REPORT;
    event.value = 0; // This value is not used but is set to 0 by convention
    write(fd, &event, sizeof(event)); // Send the event

    qDebug() << "Key sent:" << key_code << ":" << state;
}
