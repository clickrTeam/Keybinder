// TODO: rework with daemon abstract class
#include "startup.h"

QThread *deamon = nullptr;

void startUp(bool isOsStartup) {
    Profile activeProfile = proccessProfile("../../exampleProfiles/e2.json");
#ifdef _WIN32
    deamon = QThread::create(winStartDeamon, activeProfile);
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
