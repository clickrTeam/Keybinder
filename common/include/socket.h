// // #ifndef SOCKET_H
// // #define SOCKET_H

// // #include <QObject>
// // #include <QLocalServer>
// // #include <QLocalSocket>
// // #include <QByteArray>
// // #include <QDebug>

// // class MyServer : public QObject {
// //     Q_OBJECT

// //   public:
// //     MyServer();
// //     ~MyServer();

// //     void start();
// //     void stop();
// //     void onConnect(QLocalSocket* socket);
// //     void onRead();

// //   private slots:
// //     void handleNewConnection();

// //   private:
// //     QLocalServer* server;
// // };

// // #endif // SOCKET_H
// #ifndef SOCKET_H
// #define SOCKET_H

// #include <QObject>
// #include <QLocalSocket>

// class Client : public QObject
// {
//     Q_OBJECT

//   public:
//     Client(QObject *parent = nullptr);
//     ~Client() {

//     };

//   public slots:
//     void onConnected();
//     void onReadyRead();
//     void onDisconnected();
//     void onErrorOccurred(QLocalSocket::LocalSocketError error);

//   private:
//     QLocalSocket *socket;
// };

// #endif // SOCKET_H
