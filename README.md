# Clickr

## Devloped by
Bode Packer
Hayden Hilterbrand
Luke Hamling
Ryan Dalrymple
Tim Blamires

## Architecture
Keybinder
│
├── CMakeLists.txt            # CMake build file
│
├── main.cpp                  # Main entry point of the application
│
├── startup.cpp               # Startup initialization logic
│
├── ProfileInterpreter
│   └── readProfile.cpp       # Reads and processes profiles
│
├── Win
│   └── daemon.cpp            # Windows-specific daemon
│
├── Linux
│   └── daemon.cpp            # Linux-specific daemon
│
├── Mac
│   └── daemon.cpp            # Mac-specific daemon
│
└── ProfileExamples
    └── e1.json               # Example profile in JSON format
    └── e2.json               # Another example profile in JSON format
    └── ...                   # More JSON profiles

