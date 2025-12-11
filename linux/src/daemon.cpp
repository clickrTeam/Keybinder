#include "daemon.h"
#include "device_manager.h"
#include "key_channel.h"
#include "key_map.h"
#include "script_runner.h"
#include "util.h"
#include <QThread>

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
        termination_condition =
            QThread::currentThread()->isInterruptionRequested();

        struct input_event event;
        while (libevdev_next_event(keyb, LIBEVDEV_READ_FLAG_NORMAL, &event) ==
               0) {
            if (event.type == EV_KEY) {
                // Ignore repeat keys (value == 2)
                if (event.value == 2) {
                    continue;
                }

                if (int_to_keycode.contains_forward(event.code)) {
                    KeyEvent e;
                    e.keycode = int_to_keycode.find_forward(event.code);

                    if (event.value == 1) {
                        e.type = KeyEventType::Press;
                    } else {
                        e.type = KeyEventType::Release;
                    }

                    if (key_sender.send_key(e)) {
                        // Key was mapped/suppressed by the mapper
                        continue;
                    } else {
                        // Don't batch - send each event as it arrives to
                        // maintain timing
                        int key_code = int_to_keycode.find_backward(e.keycode);
                        int state = (e.type == KeyEventType::Press) ? 1 : 0;

                        // For repeat events, use state = 2
                        if (event.value == 2) {
                            state = 2;
                        }
                    }
                } else {
                    // The keycode isn't in the bimap so just forward it to the
                    // kernel
                    send_key(event.code, event.value, uinput_fd);
                }
            } else if (event.type == EV_SYN) {
                // Forward sync events to maintain proper event boundaries
                struct input_event sync_event;
                memset(&sync_event, 0, sizeof(sync_event));
                sync_event.type = EV_SYN;
                sync_event.code = SYN_REPORT;
                sync_event.value = 0;
                write(uinput_fd, &sync_event, sizeof(sync_event));
            }
        }
    }
}

void Daemon::send_outputs(const QList<OutputEvent> &outputs) {
    foreach (OutputEvent event, outputs) {
        std::visit(overloaded{
                       [&](const KeyEvent &key_event) {
                           bool type =
                               key_event.type == KeyEventType::Press ? 1 : 0;
                           int key_code =
                               int_to_keycode.find_backward(key_event.keycode);
                           send_key(key_code, type, uinput_fd);
                       },
                       [&](const RunScript &script) {
                           run_script(script.interpreter, script.script);
                       },
                   },
                   event);
    }
}

// state 1 for pressed, 0 for released
void Daemon::send_key(int key_code, int state, int fd) {
    struct input_event event;
    memset(&event, 0, sizeof(event));

    // Key press event
    event.type = EV_KEY;
    event.code = key_code;            // Key that we are sending
    event.value = state;              // 0=release, 1=press, 2=repeat
    write(fd, &event, sizeof(event)); // Send the event

    // Synchronization event
    event.type = EV_SYN;
    // SYN_REPORT -> Used to synchronize and separate events into packets
    //               of input data occurring at the same moment in time.
    event.code = SYN_REPORT;
    event.value = 0; // This value is not used but is set to 0 by convention
    write(fd, &event, sizeof(event)); // Send the event
}
