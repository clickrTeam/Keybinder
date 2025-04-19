#include "local_server.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <profile.h>

LocalServer::LocalServer(Mapper &mapper) : mapper(mapper) {
    if (!server.listen(PIPE_PATH)) {
        qFatal() << "Could not listen on: " << PIPE_PATH;
    }
    qDebug() << "Server listening on: " << PIPE_PATH;

    QObject::connect(&server, &QLocalServer::newConnection, this,
                     &LocalServer::handle_new_connection);
}

void LocalServer::handle_new_connection() {
    QLocalSocket *socket = server.nextPendingConnection();
    new ClientConnection(socket, mapper, this); // `this` for QObject parenting
}

ClientConnection::ClientConnection(QLocalSocket *socket, Mapper &mapper,
                                   QObject *parent)
    : QObject(parent), socket(socket), mapper(mapper) {
    qInfo() << "New client connected";
    connect(socket, &QLocalSocket::readyRead, this,
            &ClientConnection::read_data);
    connect(socket, &QLocalSocket::disconnected, socket,
            &QLocalSocket::deleteLater);
}

void ClientConnection::send_response(const QString &status,
                                     const QString &error) {
    QJsonObject resp;
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

        if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "JSON parse error:" << parse_error.errorString();
            send_response("fail", "invalid json");
        } else {
            QJsonObject obj = doc.object();
            const QString msg_type = obj.value("message_type").toString();

            if (msg_type == "load_profile") {
                QJsonObject profile_obj = obj.value("profile").toObject();
                try {
                    mapper.set_profile(Profile::from_json(profile_obj));
                    send_response("ok");
                } catch (const std::invalid_argument &e) {
                    qWarning() << "Invalid profile JSON:" << e.what();
                    send_response("fail", e.what());
                }
            } else {
                qWarning() << "Unknown message_type:" << msg_type;
                send_response("fail", "unknown message_type: " + msg_type);
            }
        }

        buffer.remove(0, newline_index + 1);
    }
}
