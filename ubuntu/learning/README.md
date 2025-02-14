## Dependencies
sudo apt-get install libevdev-dev (wrapper for evdev)

## Current Compile command
g++ -o detect_keyboard Detection.cpp -levdev

## Current Run Command
sudo ./detect_keyboard (need sudo to access /dev/input/eventX files)