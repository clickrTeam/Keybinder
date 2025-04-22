// This each OS should have this daemon.h file in their include path so that
// the compiler will just use the right implementation at runtime.
// See mac/include/daemon.h for an example
#include "daemon.h"
#include "local_server.h"
#include "mapper.h"
#include "read_profile.h"
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QThread>
#include <QDir>
#include <QByteArray>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QString path = "../../exampleProfiles/numberpad.json";
    if (argc < 2)
    {
        qDebug() << "Not enough arguments, using default profile location." << Qt::endl;
    }
    else
    {
        path = argv[1];
    }
    // TODO: use the first arg as profile for prototype
    Profile activeProfile =
        //Profile::from_file("../../exampleProfiles/numberpad.json");
        Profile::from_file(path);

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

    // Start the local server by calling its constructor (could add start method
    // IDK if needed)
    LocalServer server(mapper);

    // Removing for prototype as not yet used
    //
    // TODO: We need to add this argument to startup locations. i.e. add to
    // windows registery with exe location.
    // QStringList arguments = QCoreApplication::arguments();
    // bool isOsStartup = arguments.contains("--osstartup");
    //
    // if (isOsStartup) {
    //     qDebug() << "App started from system startup.";
    // } else {
    //     qDebug() << "App started manually.";
    // }
    return a.exec();
}
