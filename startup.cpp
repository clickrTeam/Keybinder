#include "startup.h"
#include "profileInterpreter/readprofile.h"
#include <qthread.h>

QThread *deamon = nullptr;

void startUp(bool isOsStartup) {
    proccessProfile("../../exampleProfiles/e1.json");
#ifdef _WIN32
    deamon = QThread::create(winStartDeamon);
#elif defined(__APPLE__)
    // TODO change
#elif defined(__linux__)
    // TODO change
#else
    #error "Unknown operating system"
#endif
    if (deamon != nullptr)
        deamon->start(QThread::HighestPriority);
    return;
}
