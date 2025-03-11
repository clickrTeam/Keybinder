#ifndef STARTUP_H
#define STARTUP_H

#include <thread>
#include "profileInterpreter/readprofile.h"
#include <qthread.h>
#include "profileInterpreter/profile.h"

#ifdef _WIN32
    #include "win/deamon.h"
#elif defined(__APPLE__)
    // TODO change
#elif defined(__linux__)
    // TODO change
#else
    #error "Unknown operating system"
#endif

void startUp(bool isOsStartup);

#endif // STARTUP_H
