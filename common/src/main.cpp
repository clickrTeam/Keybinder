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


/**
 * @brief handleSignalExit This function will handle SIGINT and SIGTERM on Linux and macOS
 *        It will write a single byte to a pipe, which is a standard and async-safe
 *        way to signal that something needs to happen. In our case, that 'something'
 *        is gracefully exiting the Qt event loop, which will be done in a connect
 *        function.
 * @param The int representing the signal
 */
void handleSignalExit(int)
{
    char c = 1;
    write(sigPipeFd[1], &c, 1);
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

    // Install signal handler for unix based systems.
    struct sigaction sa{};
    sa.sa_handler   = handleSignalExit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags     = SA_RESTART;
    sigaction(SIGINT,  &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);


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
                     [&]() {
                         // Request the QThread to stop
                         daemon_thread->requestInterruption();

                         // wait for the thread to actually finish
                         if (!daemon_thread->wait(5000)) {
                             qWarning() << "Daemon thread didn’t stop in 5s, forcing termination";
                         }

                         // Thread has stopped, clean up resources
                         daemon.cleanup();
                     });

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


    QSocketNotifier *notifier = new QSocketNotifier(sigPipeFd[0], QSocketNotifier::Read, &a);

    QObject::connect(notifier, &QSocketNotifier::activated, [&a, notifier](int) {
        notifier->setEnabled(false);        // prevent repeated triggers
        char c;
        ::read(sigPipeFd[0], &c, sizeof(c)); // clear the pipe

        // Exit the event loop
        QCoreApplication::instance()->quit();
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

    int ret = a.exec();
    return ret;
}
