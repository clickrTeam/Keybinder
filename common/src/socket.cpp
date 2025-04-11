// // #include "socket.h"

// // MyServer::MyServer() {
// //     qDebug() << "init MyServer";
// //     server = new QLocalServer(this);
// // }

// // MyServer::~MyServer() {
// //     delete server;
// // }

// // void MyServer::start() {
// //     qDebug() << "start";
// //     if (!server->listen("myapp-socket")) {
// //         qDebug() << "Failed to listen on named pipe";
// //         qDebug() << "Error:" << server->errorString();
// //     } else {
// //         qDebug() << "Listening on named pipe";
// //         connect(server, &QLocalServer::newConnection, this, &MyServer::handleNewConnection);
// //     }
// // }

// // void MyServer::stop() {
// //     server->close();
// // }

// // void MyServer::onConnect(QLocalSocket* socket) {
// //     qDebug() << "Connected!";
// // }

// // void MyServer::onRead() {
// //     qDebug() << "Read!";
// //     QLocalSocket* socket = dynamic_cast<QLocalSocket*>(sender());
// //     if (socket) {
// //         QByteArray data = socket->readAll();
// //         qDebug() << "Received data:" << data;
// //     }
// // }


// // void MyServer::handleNewConnection() {
// //     qDebug() << "handleNewConnection";
// //     QLocalSocket* socket = server->nextPendingConnection();
// //     if (socket) {
// //         connect(socket, &QLocalSocket::readyRead, this, &MyServer::onRead);
// //         connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
// //         onConnect(socket);
// //     }
// // }
// #include "socket.h"
// #include <QTextStream>
// #include <QDebug>

// Client::Client(QObject *parent) : QObject(parent)
// {
//     socket = new QLocalSocket(this);
//     connect(socket, &QLocalSocket::connected, this, &Client::onConnected);
//     connect(socket, &QLocalSocket::readyRead, this, &Client::onReadyRead);
//     connect(socket, &QLocalSocket::disconnected, this, &Client::onDisconnected);
//     connect(socket, &QLocalSocket::errorOccurred, this, &Client::onErrorOccurred);

//     QString pipeName = "mypipe";
//     QString pipePath = "\\\\.\\pipe\\" + pipeName;

//     socket->connectToServer(pipePath);
// }

// void Client::onConnected()
// {
//     qDebug() << "Client: connected to server";
//     socket->write("Hello, server!");
// }

// void Client::onReadyRead()
// {
//     qDebug() << "Client: data received from server";
//     QTextStream in(socket);
//     QString data = in.readAll();
//     qDebug() << "Client: data received:" << data;
//     socket->disconnectFromServer();
// }

// void Client::onDisconnected()
// {
//     qDebug() << "Client: disconnected from server";
// }

// void Client::onErrorOccurred(QLocalSocket::LocalSocketError error)
// {
//     qDebug() << "Client: error occurred:" << error;
// }

