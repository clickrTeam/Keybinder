#pragma once

#include "mapper.h"
#include <QByteArray>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QString>

#ifdef WIN32
constexpr auto PIPE_PATH = R"(\\.\pipe\clickr-keybinder)";
#else
constexpr auto PIPE_PATH = "/tmp/clickr_keybinder";
#endif

class LocalServer : public QObject {
    Q_OBJECT

  public:
    explicit LocalServer(Mapper &mapper);
    bool start();

  private slots:
    void handleNewConnection();

  private:
    void handleSocketData(QLocalSocket *socket);

    QLocalServer server;
    Mapper &mapper;
};
