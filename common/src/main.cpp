// This each OS should have this daemon.h file in their include path so that
// the compiler will just use the right implementation at runtime.
// See mac/include/daemon.h for an example
#include "daemon.h"
#include "local_server.h"
#include "logger.h"
#include "mapper.h"
#include "signal_handler.h"
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QThread>
#include <QByteArray>
#include <QDebug>
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <qcoreapplication.h>
#include <csignal>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
#ifdef QT_DEBUG
    QString path = "../../exampleProfiles/numberpad.json";
#else
    qInstallMessageHandler(myMessageHandler);
    QString path = "empty";
#endif
    if (argc < 2)
    {
        qDebug() << "Not enough arguments, using default profile location.";
    }
    else
    {
        path = argv[1];
    }
    Profile activeProfile =
        Profile::from_file(path);

    Signal_Handler sh;

    // Hacky workaround for circular reference
    Mapper mapper(activeProfile);
    Daemon daemon(mapper);
    mapper.set_daemon(&daemon);

    // I am not sure we will want to use qthreads in this context. A std::thread
    // may be better as it does not run an event loop which I could imagine
    // causing slowdowns
    QThread *daemon_thread = QThread::create([&] { daemon.start(); });
    daemon_thread->start(QThread::Priority::TimeCriticalPriority);
    sh.set_daemon_thread(daemon_thread);
    sh.config_handler();

    // Somehow hope this works, many varibles can make it not. Working is not so important.
    Logger logger;
    QObject::connect(QCoreApplication::instance(),
                     &QCoreApplication::aboutToQuit,
                     [&logger]() {
                         logger.cleanUp();
                     });


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

    int ret = a.exec();
    return ret;
}
