#include "daemon.h"
#include "device_manager.h"
#include "mapper.h"
#include <QThread>

Mapper *mapper = nullptr;
Daemon::Daemon(Mapper &m) {
    mapper = &m;
    // TODO: Possibly update with config file path
    event_keyb_path = retrieve_eventX();

    // No path keyb path was detected in config file
    if (event_keyb_path == "") {
        event_keyb_path = detect_keyboard();
        record_eventX(event_keyb_path);
    }
    qDebug() << "Daemon created" << Qt::endl;
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
            libevdev_grab(keyb, LIBEVDEV_UNGRAB); // Release control of the physical keyboard
            libevdev_free(keyb);
            close(keyb_fd);
        }
        qDebug() << "Daemon cleaned up" << Qt::endl;
        is_running = false;
    } else {
        qDebug()
            << "cleanup() called but daemon not running. Exiting function.";
    }
}

void Daemon::start() {
    qDebug() << "Daemon started" << Qt::endl;

    // Open the identified keyboard device in read-write mode
    int keyb_fd =
        open(event_keyb_path.toUtf8().constData(), O_RDWR | O_NONBLOCK);
    if (keyb_fd < 0) {
        qCritical() << "Failed to open device: " << strerror(errno) << Qt::endl;
        return;
    }

    struct libevdev *keyb;
    if (libevdev_new_from_fd(keyb_fd, &keyb) < 0) {
        cerr << "Failed to initialize libevdev" << endl;
        return;
    }

    // Release all keys before grab to prevent key spam
    QList<InputEvent> *key_up_events = new QList<InputEvent>();
    struct InputEvent i_event;
    for (int i = 0; i <= KEY_MAX; i++) {
        i_event.keycode = i;
        i_event.type = KeyEventType::Release;
        key_up_events->append(i_event);
    }
    send_keys_helper(*key_up_events, keyb_fd);

    // Prevent the physical keyboard key presses from being typed
    if (libevdev_grab(keyb, LIBEVDEV_GRAB) < 0) {
        cerr << "Failed to grab the keyboard device" << endl;
        return;
    }

    // Set up uinput for key injection
    uinput_fd = setup_uinput_device();
    if (uinput_fd < 0) {
        qCritical() << "Failed to set up uinput" << Qt::endl;
        return;
    }

    bool termination_condition = false;
    QList<InputEvent> event_list;
    is_running = true;
    while (!termination_condition) {
        // Check if the thread has been requested to be interrupted, if so exit
        // the loop. Otherwise this loop will block and prevent proper
        // termination
        termination_condition =
            QThread::currentThread()->isInterruptionRequested();
        struct input_event event;
        while (libevdev_next_event(keyb, LIBEVDEV_READ_FLAG_NORMAL, &event) == 0) {
            if (event.type == EV_KEY) {
                InputEvent e;
                e.keycode = event.code;
                e.type = (event.value == 1) ? KeyEventType::Press
                                            : KeyEventType::Release;

                if (mapper->map_input(e)) {
                    // Suppressed by the mapper (i.e. replaced/mapped to
                    // something else)
                    continue;
                }
                event_list.append(e);
                // Inject original key if not mapped
                send_keys(event_list);
                event_list.clear();
            }
        }
    }
}

void Daemon::send_keys(const QList<InputEvent> &vk) {
    send_keys_helper(vk, uinput_fd);
}

void Daemon::send_keys_helper(const QList<InputEvent> &vk, int fd)
{
    struct input_event event;
    memset(&event, 0, sizeof(event));
    bool type;

    foreach (InputEvent input, vk) {
        // 1 for pressed, 0 for released
        type = input.type == KeyEventType::Press ? 1 : 0;

        // Key press event
        event.type = EV_KEY;
        event.code = input.keycode; // Key that we are sending
        event.value = type; // Key up or down
        write(fd, &event, sizeof(event)); // Send the event

        // Synchronization event
        event.type = EV_SYN;
        // SYN_REPORT -> Used to synchronize and separate events into packets 
        //               of input data occurring at the same moment in time.
        event.code = SYN_REPORT;
        event.value = 0; // This value is not used but is set to 0 by convention
        write(fd, &event, sizeof(event)); // Send the event

        qDebug() << "Key sent:" << input.keycode << ":" << type << Qt::endl;
    }
}
