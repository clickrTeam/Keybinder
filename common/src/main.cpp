// This each OS should have this daemon.h file in their include path so that
// the compiler will just use the right implementation at runtime.
// See mac/include/daemon.h for an example
#include "daemon.h"
#include "mapper.h"
#include "readprofile.h"
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QThread>

int main(int argc, char *argv[]) {
    // TODO: We need to add this argument to startup locations. i.e. add to
    // windows registery with exe location.
    QCoreApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();
    Profile activeProfile = proccessProfile("../../exampleProfiles/e2.json");

    // Hacky workaround for circular reference
    Mapper mapper(activeProfile);
    Daemon daemon(mapper);
    mapper.set_daemon(&daemon);

    // I am not sure we will want to use qthreads in this context. A std::thread
    // may be better as it does not run an event loop which I could imagine
    // causing slowdowns
    QThread *daemon_thread = QThread::create([&] { daemon.start(); });
    daemon_thread->start(QThread::Priority::TimeCriticalPriority);

    QObject::connect(QCoreApplication::instance(),
                     &QCoreApplication::aboutToQuit,
                     [&]() { daemon.cleanup(); });

    bool isOsStartup = arguments.contains("--osstartup");

    if (isOsStartup) {
        qDebug() << "App started from system startup.";
    } else {
        qDebug() << "App started manually.";
    }
    return a.exec();
}
