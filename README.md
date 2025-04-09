# Clickr

## Devloped by
Bode Packer
Hayden Hilterbrand
Luke Hamling
Ryan Dalrymple
Tim Blamires

## Architecture
```
.
├── CMakeLists.txt
├── README.md
├── common
│   ├── CMakeLists.txt
│   ├── include
│   │   ├── abstract_daemon.h
│   │   ├── mapper.h
│   │   ├── profile.h
│   │   └── readprofile.h
│   └── src
│       ├── main.cpp
│       ├── mapper.cpp
│       └── readprofile.cpp
├── exampleProfiles
│   ├── e1.json
│   └── ...
├── linux
│   ├── CMakeLists.txt
│   ├── include
│   │   ├── daemon.h
│   │   └── linux_configure.h
│   ├── learning
│   │   ├── Detection.cpp
│   │   ├── README.md
│   │   ├── SimpleRemap.cpp
│   │   └── SimpleRemap.h
│   └── src
│       └── daemon.cpp
├── mac
│   ├── CMakeLists.txt
│   ├── include
│   │   └── daemon.h
│   └── src
│       └── daemon.cpp
├── startup.cpp
└── win
    ├── CMakeLists.txt
    ├── include
    │   └── daemon.h
    └── src
        └── daemon.cpp
```
