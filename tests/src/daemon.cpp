#include "daemon.h"
#include <QDebug>

Daemon::Daemon(KeySender keySender) : key_sender(keySender) {}

Daemon::~Daemon() {}

void Daemon::start() {}

void Daemon::cleanup() {}

void Daemon::send_outputs(const QList<OutputEvent> &events) {
    std::lock_guard<std::mutex> lock(mu);
    outputs.append(events);
}
QList<OutputEvent> Daemon::get_outputs() {
    std::lock_guard<std::mutex> lock(mu);
    return outputs;
}
