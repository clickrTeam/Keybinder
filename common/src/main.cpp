// This each OS should have this daemon.h file in their include path so that
// the compiler will just use the right implementation at runtime.
// See mac/include/daemon.h for an example
#include "abstract_daemon.h"
#include "readprofile.h"
#include "daemon.h"
#include "mapper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QThread>

#include <QLocalServer>
#include <QLocalSocket>
#include <QByteArray>


#include <QLocalSocket>
#include <QTextStream>
#include <QDebug>

int main(int argc, char *argv[]) {
    // TODO: We need to add this argument to startup locations. i.e. add to
    // windows registery with exe location.
    QCoreApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();
    Daemon d;
    setDaemon(&d);

    // I am not sure we will want to use qthreads in this context. A std::thread
    // may be better as it does not run an event loop which I could imagine
    // causing slowdowns
    QThread *daemon_thread = QThread::create([&] { d.start(); });
    daemon_thread->start(QThread::Priority::TimeCriticalPriority);

    QObject::connect(QCoreApplication::instance(),
                     &QCoreApplication::aboutToQuit, [&]() { d.cleanup(); });

    Profile activeProfile = proccessProfile("../../exampleProfiles/e2.json");
    setProfile(activeProfile);

#ifdef WIN32
    const QString PIPE_NAME = "mypipe";
    const QString PIPE_PATH = QString("\\\\.\\pipe\\%1").arg(PIPE_NAME);
    // Can ensure pipe server created with PS cmd:
    // (get-childitem \\.\pipe\).FullName
#else
    const QString PIPE_PATH = "/tmp/myapp-socket";
#endif
    QLocalServer server;
    if (server.listen(PIPE_PATH)) {
        qDebug() << "Server listening on pipe '\\\\\\.\\pipe\\my_pipe' or ipc socket '/tmp/myapp-socket'";
        QObject::connect(&server, &QLocalServer::newConnection, [&server]() {
            qDebug() << "new connection";
            QLocalSocket* socket = server.nextPendingConnection();
            QObject::connect(socket, &QLocalSocket::readyRead, [socket]() {
                QByteArray data = socket->readAll();
                qDebug() << "Received from Electron:" << data;
                socket->write("Hello from Qt!");
            });
        });
    }

    bool isOsStartup = arguments.contains("--osstartup");

    if (isOsStartup) {
        qDebug() << "App started from system startup.";
    } else {
        qDebug() << "App started manually.";
    }
    return a.exec();
}
