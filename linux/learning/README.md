## Dependencies
sudo apt-get install libevdev-dev (wrapper for evdev)

## Current Compile command
g++ -O2 -Wall -o detect_keyboard Detection.cpp -levdev
g++ -O2 -Wall -o simple_remap SimpleRemap.cpp -levdev

## Current Run Command
sudo ./detect_keyboard (need sudo to access /dev/input/eventX files)
sudo ./simple_remap