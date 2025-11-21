#include "local_server.h"
#include "key_counter.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtCore/qjsonobject.h>
#include <profile.h>

LocalServer::LocalServer(Mapper &mapper, KeybinderSettings &settings,
                         KeyCounter &key_counter)
    : mapper(mapper), settings(settings), key_counter(key_counter) {
    // Attempt to clean up old socket if it is still there.
    QLocalServer::removeServer(PIPE_PATH);

    if (!server.listen(PIPE_PATH)) {
        qFatal() << "Could not listen on: " << PIPE_PATH;
    }
    qDebug() << "Server listening on: " << PIPE_PATH;

    // Set socket permissions
    QFile socketFile(PIPE_PATH);
    socketFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
                              QFile::ReadGroup | QFile::WriteGroup |
                              QFile::ReadOther | QFile::WriteOther);

    QObject::connect(&server, &QLocalServer::newConnection, this,
                     &LocalServer::handle_new_connection);
}

LocalServer::~LocalServer() {
    qDebug() << "Entering LocalServer destructor";
    server.close();
    QLocalServer::removeServer(PIPE_PATH);
}

void LocalServer::handle_new_connection() {
    QLocalSocket *socket = server.nextPendingConnection();
    new ClientConnection(socket, mapper, settings, key_counter,
                         this); // `this` for QObject parenting
}

ClientConnection::ClientConnection(QLocalSocket *socket, Mapper &mapper,
                                   KeybinderSettings &settings,
                                   KeyCounter &key_counter, LocalServer *parent)
    : QObject(parent), socket(socket), mapper(mapper), settings(settings),
      key_counter(key_counter) {
    qInfo() << "New client connected";
    connect(this, &ClientConnection::pause_requested, parent,
            &LocalServer::pause_requested);
    connect(this, &ClientConnection::resume_requested, parent,
            &LocalServer::resume_requested);
    connect(socket, &QLocalSocket::readyRead, this,
            &ClientConnection::read_data);
    connect(socket, &QLocalSocket::disconnected, socket,
            &QLocalSocket::deleteLater);
}

void ClientConnection::send_response(const QString &status,
                                     const QString &error,
                                     std::optional<QJsonObject> rest) {
    QJsonObject resp = rest.value_or(QJsonObject());
    resp["status"] = status;
    if (status == "fail") {
        resp["error"] = error;
    }
    auto doc = QJsonDocument(resp);
    socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void ClientConnection::read_data() {
    qInfo() << "New message from client";

    buffer.append(socket->readAll());

    int newline_index;
    // TODO use more helper functions
    while ((newline_index = buffer.indexOf('\n')) != -1) {
        QByteArray line = buffer.left(newline_index);
        if (!line.isEmpty() && line.endsWith('\r'))
            line.chop(1);

        // parse JSON
        QJsonParseError parse_error;
        QJsonDocument doc = QJsonDocument::fromJson(line, &parse_error);
        qDebug() << "Loading message from electron:" << doc;

        if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "JSON parse error:" << parse_error.errorString();
            send_response("fail", "invalid json");
        } else {
            QJsonObject obj = doc.object();
            const QString msg_type = obj.value("type").toString();

            if (msg_type == "load_profile") {
                QJsonObject profile_obj = obj.value("profile").toObject();
                try {
                    mapper.set_profile(Profile::from_json(profile_obj));
                    send_response("ok");
                } catch (const std::invalid_argument &e) {
                    qWarning() << "Invalid profile JSON:" << e.what();
                    send_response("fail", e.what());
                }
            } else if (msg_type == "set_settings") {
                if (settings.load_from_json(obj["settings"].toObject())) {
                    send_response("ok");
                } else {
                    send_response("fail", "invalid settings JSON");
                }

            } else if (msg_type == "get_frequencies") {
                send_response(
                    "success", "",
                    QJsonObject({{"frequencies", key_counter.to_json()}}));
            } else if (msg_type == "pause") {
                emit pause_requested();
                send_response("success", "");
            } else if (msg_type == "resume") {
                emit resume_requested();
                send_response("success", "");
            } else {
                qWarning() << "Unknown message_type:" << msg_type;
                send_response("fail", "unknown message_type: " + msg_type);
            }
        }

        buffer.remove(0, newline_index + 1);
    }
}
