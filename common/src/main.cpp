// This each OS should have this daemon.h file in their include path so that
// the compiler will just use the right implementation at runtime.
// See mac/include/daemon.h for an example
#include "daemon.h"
#include "local_server.h"
#include "logger.h"
#include "mapper.h"
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

// Testing pipe method
#include <QSocketNotifier>
#include <QTimer>
static int sigPipeFd[2];
/*
 *  Signal handler: forward SIGINT/SIGTERM to Qt event loop.
 *  This will end the Qt event loop, things will go out of scope as expected,
 *  destructors will be called and resources will be cleaned up.
 */
void handleSignalExit(int)
{
    qDebug() << "Inside handleSignalExit";
//    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit",
//                              Qt::QueuedConnection);

    char c = 1;
    write(sigPipeFd[1], &c, 1);
//    QCoreApplication::quit();
    qDebug() << "After write to pipe in sighandler";
}


void handleSignal(QSocketNotifier* notifier, QCoreApplication* app) {
    qDebug() << "Inside signal handler with notifier";
    notifier->setEnabled(false);
    char c;
    ::read(sigPipeFd[0], &c, sizeof(c));
    app->quit();
}


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


    // Create a pipe
    if (pipe(sigPipeFd)) {
        qFatal("Failed to create pipe");
    }

    // 1a. Log the main thread’s ID
    Qt::HANDLE mainTid = QThread::currentThreadId();
    qDebug() << "[main] QThread::currentThread() =" << QThread::currentThread()
             << " tid =" << mainTid;

    // Install signal handler
    // std::signal(SIGINT, handleSignalExit); // kill -2 or Ctrl+C
    struct sigaction sa{};
    sa.sa_handler   = handleSignalExit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags     = SA_RESTART;
    sigaction(SIGINT,  &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);


    // Hacky workaround for circular reference
//    Mapper mapper(activeProfile);
//    Daemon daemon(mapper);
//    mapper.set_daemon(&daemon);

    // I am not sure we will want to use qthreads in this context. A std::thread
    // may be better as it does not run an event loop which I could imagine
    // causing slowdowns
//    QThread *daemon_thread = QThread::create([&] { daemon.start(); });
//    daemon_thread->start(QThread::Priority::TimeCriticalPriority);

//    QObject::connect(QCoreApplication::instance(),
//                     &QCoreApplication::aboutToQuit,
//                     [&]() { daemon.cleanup(); });

    // Somehow hope this works, many varibles can make it not. Working is not so important.
    Logger logger;
    QObject::connect(QCoreApplication::instance(),
                     &QCoreApplication::aboutToQuit,
                     [&logger]() {
                         logger.cleanUp();
                     });


    // Start the local server by calling its constructor (could add start method
    // IDK if needed)
//    LocalServer server(mapper);


    /// @todo Testing pipe method, should work for linux and mac but currently does not.
    QSocketNotifier *notifier = new QSocketNotifier(sigPipeFd[0], QSocketNotifier::Read, &a);

    /// @todo This is not getting executed when the pipe is written to.
    QObject::connect(notifier, &QSocketNotifier::activated, [&a, notifier](int) {
        notifier->setEnabled(false);        // prevent repeated triggers
        char c;
        ::read(sigPipeFd[0], &c, sizeof(c)); // clear the pipe
        qDebug() << "About to quit app, notifier has been activated";
        Qt::HANDLE slotTid = QThread::currentThreadId();
        qDebug() << "[slot] QThread::currentThread() ="
                 << QThread::currentThread()
                 << " tid =" << slotTid;
        // attempt 1
        QCoreApplication::instance()->quit();
        //a.quit();                             // gracefully exit the event loop
        qDebug() << "a.quit() was called in connect";

        // attempt2
//        bool was_invoked = QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);
//        qDebug() << "Invoked quit method via queued connection returned " << was_invoked;
        // attempt 3
//        QTimer::singleShot(0, &a, &QCoreApplication::quit);
//        qDebug() << "Invoked quit method with singleshot timer";
    });


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

    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, []() {
        qInfo() << "aboutToQuit emitted";
    });

    //return a.exec();
    qDebug() << "Entering exec()";
    int ret = a.exec();
    qDebug() << "Returned from exec(), code =" << ret;

    return ret;
}
