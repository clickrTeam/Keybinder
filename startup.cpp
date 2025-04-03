#include "startup.h"
#include "./ubuntu/learning/SimpleRemap.h"
//#include "./ubuntu/learning/SimpleRemap.cpp"
#include "mapper.h"
QThread *deamon = nullptr;

void startUp(bool isOsStartup) {
    Profile loadedProfile = proccessProfile("../../exampleProfiles/e2.json");
    setProfile(loadedProfile);
#ifdef _WIN32
    deamon = QThread::create(winStartDeamon);
#elif defined(__APPLE__)
    // TODO change
#elif defined(__linux__)
    deamon = QThread::create(linuxStartDeamon);
#else
    #error "Unknown operating system"
#endif
    if (deamon != nullptr)
        deamon->start(QThread::HighestPriority);
    return;
}
