#include "local_server.h"

LocalServer::LocalServer(Mapper &mapper) : mapper(mapper) {

    // TODO: remove old sockets on linux and mac
    if (!server.listen(PIPE_PATH)) {
        qFatal() << "Could not listen on: " << PIPE_PATH;
    }
    qDebug() << "Server listening on: " << PIPE_PATH;
    QObject::connect(
        &server, &QLocalServer::newConnection, [&server, &mapper]() {
            qDebug() << "new connection";
            QLocalSocket *socket = server.nextPendingConnection();
            QObject::connect(
                socket, &QLocalSocket::readyRead, [socket, &mapper]() {
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

#include "local_server.h"

LocalServer::LocalServer(Mapper &mapper) : mapper(mapper) {
    if (!server.listen(PIPE_PATH)) {
        qFatal() << "Could not listen on: " << PIPE_PATH;
    }
    qDebug() << "Server listening on: " << PIPE_PATH;

    QObject::connect(&server, &QLocalServer::newConnection, this,
                     &LocalServer::handleNewConnection);
}

void LocalServer::handleNewConnection() {
    qDebug() << "New connection received";
    QLocalSocket *socket = server.nextPendingConnection();
    QObject::connect(socket, &QLocalSocket::readyRead,
                     [this, socket]() { handleSocketData(socket); });
}

void LocalServer::handleSocketData(QLocalSocket *socket) {
    QByteArray data = socket->readAll();
    qDebug() << "Received from client:" << data;

    if (data.startsWith("start")) {
        qDebug() << "ERR: daemon already started";
        socket->write("ERR: daemon already started");
    } else if (data.startsWith("load:")) {
        data.remove(0, 5); // Remove "load:"
        Profile p = readProfile(QJsonDocument::fromJson(data));
        mapper.set_profile(&p);
    }
}
