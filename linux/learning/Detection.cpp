#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <string.h>
#include <unistd.h>
#include <vector>
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main() {
    // Open the /dev/input/ directory
    DIR *dir = opendir("/dev/input/");
    if (!dir) {
        std::cerr << "Failed to open /dev/input/ directory" << std::endl;
        return 1;
    }

    struct dirent *entry;
    vector<string> possible_keyb_paths;
    vector<int> fds;
    vector<libevdev *> devices;

    int event_counter = 0;
    int possible_keyboards = 0;

    // Loop over each entry in /dev/input/
    while ((entry = readdir(dir)) != nullptr) {
        // Only process "eventX" files
        if (strncmp(entry->d_name, "event", 5) == 0) {
            string event_path = "/dev/input/" + string(entry->d_name);
            event_counter++;
            // cout << event_path << endl;

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

            // If a keyboard device has spacebar and the x key, it is most
            // likely an actual keyboard
            bool has_key_events =
                libevdev_has_event_type(dev, EV_KEY) &&
                libevdev_has_event_code(dev, EV_KEY, KEY_SPACE) &&
                libevdev_has_event_code(dev, EV_KEY, KEY_X);
            if (has_key_events) {
                possible_keyb_paths.push_back(event_path);
                cout << "possible keyb path: " + event_path
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
    // On my desktop, I get 5 devices. On my laptop, I only get 1 and it is the
    // correct device.
    cout << "Number of /dev/input/eventX devices: " << event_counter << endl;
    cout << "Number of possible keyboards: " << possible_keyboards << endl;

    // Now we listen to key events to determine which device is indeed the
    // keyboard
    cout << "Press SPACEBAR to identify the correct keyboard device." << endl;
    bool found_keyb = false;
    string keyb_path = "";

    while (!found_keyb) {
        for (size_t i = 0; i < devices.size(); i++) {
            struct input_event event;
            while (libevdev_next_event(devices[i], LIBEVDEV_READ_FLAG_NORMAL,
                                       &event) == 0) {
                if (event.type == EV_KEY && event.code == KEY_SPACE &&
                    event.value == 1) // Space has been pressed on devices[i]
                {
                    cout << "Detected spacebar press on: "
                         << possible_keyb_paths[i] << endl;
                    keyb_path = possible_keyb_paths[i];
                    found_keyb = true;
                    break;
                }
            }
        }

        if (found_keyb) {
            break;
        }
    }

    // Clean up all devices
    for (size_t i = 0; i < devices.size(); i++) {
        libevdev_free(devices[i]);
        close(fds[i]);
    }
    devices.clear();
    fds.clear();

    // Re-open keyb now that we know proper device. It must be reopened because
    // of the library.
    int keyb_fd = open(keyb_path.c_str(), O_RDONLY | O_NONBLOCK);
    if (keyb_fd < 0) {
        cerr << "Failed to open device: " << strerror(errno) << endl;
        return 1;
    }

    struct libevdev *keyb;
    if (libevdev_new_from_fd(keyb_fd, &keyb) < 0) {
        std::cerr << "Failed to initialize libevdev" << std::endl;
        return 1;
    }

    // Detect other key presses
    bool escape_pressed = false;
    cout << "\nMonitoring key presses. Press ESC to terminate." << endl;

    while (!escape_pressed) {
        struct input_event event;

        while (libevdev_next_event(keyb, LIBEVDEV_READ_FLAG_NORMAL, &event) ==
               0) {
            if (event.type == EV_KEY &&
                event.value == 1) // ESC has been pressed
            {
                cout << "User pressed key: "
                     << libevdev_event_code_get_name(EV_KEY, event.code)
                     << endl;

                if (event.code == KEY_ESC) {
                    escape_pressed = true;
                    break;
                }
            }
        }

        if (escape_pressed) {
            break;
        }
    }
    return 0;
}