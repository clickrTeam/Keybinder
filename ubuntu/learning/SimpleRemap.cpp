#include <linux/uinput.h>  // Required for injecting events
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <vector>
using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::vector;


// Open the uinput device to send key events
int setup_uinput_device()
{
    int uinp_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinp_fd < 0) {
        cerr << "Failed to open /dev/uinput" << endl;
        return -1;
    }

    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_SYN);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_H);

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
        return -1;
    }

    return uinp_fd;
}

// Send key press and release events for 'h'
void send_key_event(int fd, int keycode)
{
    struct input_event event;
    memset(&event, 0, sizeof(event));

    // Key press event
    event.type = EV_KEY;
    event.code = keycode;
    event.value = 1;
    write(fd, &event, sizeof(event));

    // Key release event
    event.value = 0;
    write(fd, &event, sizeof(event));

    // Synchronization event
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(fd, &event, sizeof(event));
}

int main()
{
    // Open the /dev/input/ directory
    DIR *dir = opendir("/dev/input/");
    if (!dir) {
        cerr << "Failed to open /dev/input/ directory" << endl;
        return 1;
    }

    struct dirent *entry;
    vector<string> possible_keyb_paths;
    vector<int> fds;
    vector<libevdev*> devices;

    int event_counter = 0;
    int possible_keyboards = 0;

    // Loop over each entry in /dev/input/
    while ((entry = readdir(dir)) != nullptr) 
    {
        // Only process "eventX" files
        if (strncmp(entry->d_name, "event", 5) == 0) 
        {
            string event_path = "/dev/input/" + string(entry->d_name);
            event_counter++;

            struct libevdev *dev = nullptr;

            int fd = open(event_path.c_str(), O_RDONLY | O_NONBLOCK);
            if (fd < 0) {
                cerr << "Failed to open " << event_path << endl;
                return false;
            }
        
            // Initialize the evdev device
            if (libevdev_new_from_fd(fd, &dev) < 0) {
                cerr << "Failed to initialize evdev device" << endl;
                close(fd);
                return false;
            }

            // If a keyboard device has spacebar and the x key, it is most likely an actual keyboard
            bool has_key_events = libevdev_has_event_type(dev, EV_KEY) && libevdev_has_event_code(dev, EV_KEY, KEY_SPACE) && libevdev_has_event_code(dev, EV_KEY, KEY_X);
            if(has_key_events)
            {
                possible_keyb_paths.push_back(event_path);
                cout << "possible keyb path: " + event_path << endl;  // Still need to verify that this is actually a keyboard
                possible_keyboards++;
                fds.push_back(fd);
                devices.push_back(dev);
            }
            else
            {
                // Clean up
                libevdev_free(dev);
                close(fd);
            }
        }
    }
    // On my desktop, I get 5 devices. On my laptop, I only get 1 and it is the correct device.
    cout << "Number of /dev/input/eventX devices: " << event_counter << endl;
    cout << "Number of possible keyboards: " << possible_keyboards << endl;

    // Now we listen to key events to determine which device is indeed the keyboard
    cout << "Press SPACEBAR to identify the correct keyboard device." << endl;
    bool found_keyb = false;
    string keyb_path = "";
    
    while(!found_keyb)
    {
        for(size_t i = 0; i < devices.size(); i++)
        {
            struct input_event event;
            while (libevdev_next_event(devices[i], LIBEVDEV_READ_FLAG_NORMAL, &event) == 0) 
            {
                if (event.type == EV_KEY && event.code == KEY_SPACE && event.value == 1) // Space has been pressed on devices[i]
                {
                    cout << "Detected spacebar press on: " << possible_keyb_paths[i] << endl;
                    keyb_path = possible_keyb_paths[i];
                    found_keyb = true;
                    break;
                }
            }
        }

        if (found_keyb)
        {
            break;
        }
    }

    // Clean up all devices
    for(size_t i = 0; i < devices.size(); i++)
    {
        libevdev_free(devices[i]);
        close(fds[i]);
    }
    devices.clear();
    fds.clear();

    // Open the identified keyboard device in read-write mode
    int keyb_fd = open(keyb_path.c_str(), O_RDWR | O_NONBLOCK);
    if (keyb_fd < 0) 
    {
        cerr << "Failed to open device: " << strerror(errno) << endl;
        return 1;
    }

    struct libevdev *keyb;
    if (libevdev_new_from_fd(keyb_fd, &keyb) < 0) 
    {
        cerr << "Failed to initialize libevdev" << endl;
        return 1;
    }

    // Prevent the original key press from being typed
    if (libevdev_grab(keyb, LIBEVDEV_GRAB) < 0) {
        cerr << "Failed to grab the keyboard device" << endl;
        return 1;
    }    

    // Set up uinput for key injection
    int uinp_fd = setup_uinput_device();
    if (uinp_fd < 0)
    {
        cerr << "Failed to set up uinput" << endl;
        return 1;
    }

    bool escape_pressed = false;
    int key = KEY_H;

    cout << "\nRemapping keys to '" << key << "'. Press ESC to terminate." << endl;

    while (!escape_pressed)
    {
        struct input_event event;
        while (libevdev_next_event(keyb, LIBEVDEV_READ_FLAG_NORMAL, &event) == 0) 
        {
            if (event.type == EV_KEY && event.value == 1) 
            {
                if (event.code == KEY_ESC) 
                {
                    escape_pressed = true;
                    break;
                }
                // Send new key instead of the original key
                send_key_event(uinp_fd, key);
            }
        }
    }

    // Cleanup
    ioctl(uinp_fd, UI_DEV_DESTROY);
    close(uinp_fd);
    libevdev_grab(keyb, LIBEVDEV_UNGRAB);
    libevdev_free(keyb);
    close(keyb_fd);

    return 0;
}
