#include "daemon.h"
#include "linux_configure.h"
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

    // Prevent the original key press from being typed
    if (libevdev_grab(keyb, LIBEVDEV_GRAB) < 0) {
        cerr << "Failed to grab the keyboard device" << endl;
        return;
    }

    // Set up uinput for key injection
    setup_uinput_device();
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
        while (libevdev_next_event(keyb, LIBEVDEV_READ_FLAG_NORMAL, &event) ==
               0) {

            if (event.type == EV_KEY) {
                // TODO: Send key code to mapper.cpp and have it determine which
                // key / combo to output.
                //                if (event.code == KEY_ESC)
                //                {
                //                    escape_pressed = true;
                //                    break;
                //                }
                //                // Send new key instead of the original key
                //                for(Keybind bind :
                //                profile.layers.first().keybinds)
                //                {
                //                    if(bind.bind.value == event.code)
                //                    {
                //                        qDebug() << "Sending mapped key: " <<
                //                        bind.key.value;
                //                        send_key_event(uinp_fd,
                //                        map.getKey(bind.key.value));
                //                    }
                //                }
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

void Daemon::cleanup() {
    if (this->is_running) {
        ioctl(uinput_fd, UI_DEV_DESTROY);
        close(uinput_fd);
        if (keyb != nullptr && keyb_fd >= 0) {
            libevdev_grab(keyb, LIBEVDEV_UNGRAB); // @todo crash here
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

void Daemon::send_keys(const QList<InputEvent> &vk) {
    struct input_event event;
    bool type;
    memset(&event, 0, sizeof(event));

    foreach (InputEvent input_event, vk) {
        type = input_event.type == KeyEventType::Press ? 1 : 0;

        // Key press event
        event.type = EV_KEY;
        event.code = input_event.keycode;
        event.value = type;
        write(uinput_fd, &event, sizeof(event));

        // Key release event
        // event.value = 0;
        // write(uinput_fd, &event, sizeof(event));

        // Synchronization event
        event.type = EV_SYN;
        event.code = SYN_REPORT;
        event.value = 0;
        write(uinput_fd, &event, sizeof(event));

        qDebug() << "Key sent:" << input_event.keycode << ":" << type
                 << Qt::endl;
    }
}

/**
 * @brief Set up a virtual keyboard device using the Linux uinput subsystem.
 *
 * This function opens /dev/uinput and registers a new virtual input device
 * that behaves like a keyboard. It enables support for key events (EV_KEY)
 * and synchronization events (EV_SYN), then marks all possible key codes
 * (0..KEY_MAX) as supported. The device is given a USB-like identity and a
 * descriptive name ("clickr_virtual_keyboard"), and is created in the kernel
 * so it appears as an input device (e.g., /dev/input/eventX).
 *
 * On success, the file descriptor for the created uinput device is stored
 * in the class member variable `uinput_fd` for later use when injecting
 * key events. On failure, the function prints an error and returns without
 * creating a device.
 */
void Daemon::setup_uinput_device() {
    int uinp_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinp_fd < 0) {
        qCritical() << "Failed to open /dev/uinput" << Qt::endl;
        return;
    }

    // Declare which event types the keyboard supports.
    // EV_KEY -> This device can generate key events (keyboard keys, mouse, buttons).
    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    // EV_SYN -> Synchronization events, used to mark the end of a batch of events.
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
    strcpy(uinput_device.name, "clickr_virtual_keyboard"); 


    if (ioctl(uinp_fd, UI_DEV_SETUP, &uinput_device) < 0 || // Configure the device with the provided info
        ioctl(uinp_fd, UI_DEV_CREATE) < 0) { // Tells the kernel to create the device
        qCritical() << "Failed to create uinput device" << Qt::endl;
        close(uinp_fd);
        return;
    }

    // Save the file descriptor
    uinput_fd = uinp_fd;
}
