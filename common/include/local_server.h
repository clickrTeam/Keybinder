#pragma once

#include "mapper.h"
#include "settings.h"
#include <QByteArray>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QString>

#ifdef WIN32
constexpr auto PIPE_PATH = R"(\\.\pipe\clickr)";
#else
constexpr auto PIPE_PATH = "/tmp/clickr.sock";
#endif

class LocalServer : public QObject {
    Q_OBJECT

  public:
    explicit LocalServer(Mapper &mapper, KeybinderSettings &settings);
    virtual ~LocalServer();
    bool start();

  private slots:
    void handle_new_connection();

  private:
    void handleSocketData(QLocalSocket *socket);

    QLocalServer server;
    Mapper &mapper;
    KeybinderSettings &settings;
};

class ClientConnection : public QObject {
    Q_OBJECT

  public:
    ClientConnection(QLocalSocket *socket, Mapper &mapper,
                     KeybinderSettings &settings, QObject *parent = nullptr);
    virtual ~ClientConnection() = default;

  private slots:
    void read_data();

  private:
    void send_response(const QString &status, const QString &error = QString());

    QLocalSocket *socket;
    Mapper &mapper;
    KeybinderSettings &settings;
    QByteArray buffer;
};
