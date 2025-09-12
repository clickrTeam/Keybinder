// This each OS should have this daemon.h file in their include path so that
// the compiler will just use the right implementation at runtime.
// See mac/include/daemon.h for an example.
#include "daemon.h"
#include "key_channel.h"
#include "local_server.h"
#include "logger.h"
#include "mapper.h"
#include "read_profile.h"
#include "signal_handler.h"
#include <QApplication>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QStringList>
#include <QTextStream>
#include <QThread>
#include <csignal>
#include <qcoreapplication.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QString path = "empty";
    Profile activeProfile;

#ifdef QT_DEBUG
    // path = "../../exampleProfiles/numberpad.json";
#else
    qInstallMessageHandler(myMessageHandler);
#endif
    if (argc < 2) {
        qDebug() << "Not enough arguments, using default profile location.";
    } else {
        path = argv[1];
    }

    if (path == "empty") {
        activeProfile = Profile::loadLatest();
    } else {
        activeProfile = Profile::from_file(path);
    }

    SignalHandler sh;

    auto [sender, receiver] = create_channel();
    Daemon daemon(sender);
    Mapper mapper(activeProfile, daemon, receiver);

    // I am not sure we will want to use qthreads in this context. A std::thread
    // may be better as it does not run an event loop which I could imagine
    // causing slowdowns
    QThread *daemon_thread = QThread::create([&] { daemon.start(); });
    QThread *mapper_thread = QThread::create([&] { mapper.start(); });
    daemon_thread->start(QThread::Priority::TimeCriticalPriority);
    mapper_thread->start(QThread::Priority::TimeCriticalPriority);
    sh.set_daemon_thread(daemon_thread);
    sh.config_handler();

    // Somehow hope this works, many varibles can make it not. Working is not so
    // important.
    Logger logger;
    QObject::connect(QCoreApplication::instance(),
                     &QCoreApplication::aboutToQuit,
                     [&logger]() { logger.cleanUp(); });

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
