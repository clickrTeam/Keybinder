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

#include <QLocalServer>
#include <QByteArray>
#include <QLocalSocket>
#include <QDebug>

int main(int argc, char *argv[]) {
    // TODO: We need to add this argument to startup locations. i.e. add to
    // windows registery with exe location.
    QCoreApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();
    Profile activeProfile = proccessProfile("../../exampleProfiles/e4.json");

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
        QObject::connect(&server, &QLocalServer::newConnection, [&server, &mapper]() {
            qDebug() << "new connection";
            QLocalSocket* socket = server.nextPendingConnection();
            QObject::connect(socket, &QLocalSocket::readyRead, [socket, &mapper]() {
                QByteArray data = socket->readAll();
                qDebug() << "Received from Electron:" << data;
                if (data.startsWith("start")) {
                    qDebug() << "ERR: daemon already started";
                    socket->write("ERR: daemon already started");
                    return;
                } else if (data.startsWith("load:")) {
                    data.remove(0, 5); // Remove load:
                    Profile p = readProfile(QJsonDocument::fromJson(data));
                    mapper.set_profile(&p);
                }
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
