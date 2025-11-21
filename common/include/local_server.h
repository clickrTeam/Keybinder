#pragma once

#include "key_counter.h"
#include "mapper.h"
#include "settings.h"
#include <QByteArray>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QtCore/qjsonobject.h>

#ifdef WIN32
constexpr auto PIPE_PATH = R"(\\.\pipe\clickr)";
#else
constexpr auto PIPE_PATH = "/tmp/clickr.sock";
#endif

class LocalServer : public QObject {
    Q_OBJECT

  public:
    explicit LocalServer(Mapper &mapper, KeybinderSettings &settings,
                         KeyCounter &key_counter);
    virtual ~LocalServer();
    bool start();
  signals:
    void pause_requested();
    void resume_requested();

  private slots:
    void handle_new_connection();

  private:
    void handleSocketData(QLocalSocket *socket);

    QLocalServer server;
    Mapper &mapper;
    KeybinderSettings &settings;
    KeyCounter &key_counter;
};

class ClientConnection : public QObject {
    Q_OBJECT

  public:
    ClientConnection(QLocalSocket *socket, Mapper &mapper,
                     KeybinderSettings &settings, KeyCounter &key_counter,
                     LocalServer *parent = nullptr);
    virtual ~ClientConnection() = default;

  signals:
    void pause_requested();
    void resume_requested();

  private slots:
    void read_data();

  private:
    void send_response(const QString &status, const QString &error = QString(),
                       std::optional<QJsonObject> rest = std::nullopt);

    QLocalSocket *socket;
    Mapper &mapper;
    KeybinderSettings &settings;
    KeyCounter &key_counter;
    QByteArray buffer;
};
