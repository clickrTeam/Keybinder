#include "socket.h"
#include <QDebug>

MyServer::MyServer(QObject *parent) : QObject(parent)
{
    server = new QLocalServer(this);
    connect(server, &QLocalServer::newConnection, this, &MyServer::handleConnection);
    if (!server->listen("myapp-socket")) {
        qDebug() << "Failed to listen on named pipe";
    }
}

MyServer::~MyServer()
{
    delete server;
}

void MyServer::handleConnection()
{
    QLocalSocket *socket = server->nextPendingConnection();
    if (socket) {
        connect(socket, &QLocalSocket::readyRead, this, &MyServer::handleReadyRead);
        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
    }
}

void MyServer::handleReadyRead()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (socket) {
        QByteArray data = socket->readAll();
        // Process the data here
        qDebug() << "Received data:" << data;
        // Send a response back to the client
        socket->write("Response from server\n");
    }
}
