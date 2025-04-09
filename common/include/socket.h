#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QByteArray>

class MyServer : public QObject
{
    Q_OBJECT

  public:
    MyServer(QObject *parent = nullptr);
    ~MyServer();

  public slots:
    void handleConnection();
    void handleReadyRead();

  private:
    QLocalServer *server;
};

#endif // SOCKET_H
