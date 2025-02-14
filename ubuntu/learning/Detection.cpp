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

int main()
{
    // Open the /dev/input/ directory
    DIR *dir = opendir("/dev/input/");
    if (!dir) {
        std::cerr << "Failed to open /dev/input/ directory" << std::endl;
        return 1;
    }

    struct dirent *entry;
    vector<string> possible_keyb_paths;
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
            //cout << event_path << endl;

            struct libevdev *dev = nullptr;

            int fd = open(event_path.c_str(), O_RDONLY);
            if (fd < 0) {
                std::cerr << "Failed to open " << event_path << std::endl;
                return false;
            }
        
            // Initialize the evdev device
            if (libevdev_new_from_fd(fd, &dev) < 0) {
                std::cerr << "Failed to initialize evdev device" << std::endl;
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
            }
            // Clean up
            libevdev_free(dev);
            close(fd);
        }
    }
    // On my desktop, I get 5 devices. On my laptop, I only get 1 and it is the correct device.
    cout << "Number of /dev/input/eventX devices: " << event_counter << endl;
    cout << "Number of possible keyboards: " << possible_keyboards << endl;
    return 0;
}