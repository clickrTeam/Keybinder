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
    void send_keys(const QList<InputEvent> &events) override;

    KeySender key_sender;
};
