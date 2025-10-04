#pragma once

#include "abstract_daemon.h"
#include "key_channel.h"

class Daemon : AbstractDaemon {

  public:
    // Constructor and Destructor
    Daemon(KeySender);
    ~Daemon();

    // Override abstract class methods
    void start() override;
    void cleanup() override;

    void send_outputs(const QList<OutputEvent> &events) override;
    QList<OutputEvent> get_outputs();

    KeySender key_sender;

  private:
    std::mutex mu;
    QList<OutputEvent> outputs;
};
