#include "daemon.h"
#include "linux_configure.h"
#include "mapper.h"

Mapper* mapper = nullptr;
Daemon::Daemon(Mapper &m)
{
    mapper = &m;
    //TODO: Possibly update with config file path
    event_keyb_path = retrieve_eventX();

    // No path keyb path was detected in config file
    if(event_keyb_path == "")
    {
        event_keyb_path = detect_keyboard();
        record_eventX(event_keyb_path);
    }
    qDebug() << "Daemon created" << Qt::endl;
    // Do stuff
}

Daemon::~Daemon()
{
    cleanup();
    qDebug() << "Daemon destroyed" << Qt::endl;
}

void Daemon::start()
{
    qDebug() << "Daemon started" << Qt::endl;

    // Open the identified keyboard device in read-write mode
    int keyb_fd = open(event_keyb_path.toUtf8().constData(), O_RDWR | O_NONBLOCK);
    if (keyb_fd < 0)
    {
        qCritical() << "Failed to open device: " << strerror(errno) << Qt::endl;
        return;
    }

    struct libevdev *keyb;
    if (libevdev_new_from_fd(keyb_fd, &keyb) < 0)
    {
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
    if (uinput_fd < 0)
    {
        qCritical() << "Failed to set up uinput" << Qt::endl;
        return;
    }

    bool termination_condition = false; //TODO: A signal from the Electron app? Something to say 'stop the daemon'.
    QList<InputEvent> event_list;

    while (!termination_condition)
    {
        struct input_event event;
        while (libevdev_next_event(keyb, LIBEVDEV_READ_FLAG_NORMAL, &event) == 0)
        {

            if (event.type == EV_KEY)
            {
                //TODO: Send key code to mapper.cpp and have it determine which key / combo to output.
//                if (event.code == KEY_ESC)
//                {
//                    escape_pressed = true;
//                    break;
//                }
//                // Send new key instead of the original key
//                for(Keybind bind : profile.layers.first().keybinds)
//                {
//                    if(bind.bind.value == event.code)
//                    {
//                        qDebug() << "Sending mapped key: " << bind.key.value;
//                        send_key_event(uinp_fd, map.getKey(bind.key.value));
//                    }
//                }
                InputEvent e;
                e.keycode = event.code;
                e.type = (event.value == 1) ? KeyEventType::Press : KeyEventType::Release;

                if (mapper->map_input(e)) {
                    // Suppressed by the mapper (i.e. replaced/mapped to something else)
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

void Daemon::cleanup()
{
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
    libevdev_grab(keyb, LIBEVDEV_UNGRAB);
    libevdev_free(keyb);
    close(keyb_fd);

    qDebug() << "Daemon cleaned up" << Qt::endl;
}

void Daemon::send_keys(const QList<InputEvent> &vk)
{
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
        //event.value = 0;
        //write(uinput_fd, &event, sizeof(event));

               // Synchronization event
        event.type = EV_SYN;
        event.code = SYN_REPORT;
        event.value = 0;
        write(uinput_fd, &event, sizeof(event));

        qDebug() << "Key sent:" << input_event.keycode << ":" << type << Qt::endl;
    }
}

void Daemon::setup_uinput_device()
{
    int uinp_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinp_fd < 0)
    {
        cerr << "Failed to open /dev/uinput" << endl;
        return;
    }

    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_SYN);

    // TODO: Should not need this part if we are handling mapping with mapper.cpp
   // Registers all of the keys to be remapped
    for (int i = 0; i < 128; i++)
    {
        ioctl(uinp_fd, UI_SET_KEYBIT, i);
    }


    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1;
    usetup.id.product = 0x1;
    strcpy(usetup.name, "h_key_mapper");

    if (ioctl(uinp_fd, UI_DEV_SETUP, &usetup) < 0 ||
        ioctl(uinp_fd, UI_DEV_CREATE) < 0) {
        cerr << "Failed to create uinput device" << endl;
        close(uinp_fd);
        return;
    }

    uinput_fd = uinp_fd;
}
