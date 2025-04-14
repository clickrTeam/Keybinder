#pragma once
#include "abstract_daemon.h"
#include "mapper.h"

class Daemon : public AbstractDaemon {
  public:
    // Constructor and Destructor
    Daemon(Mapper &m);
    ~Daemon();

    // Override abstract class methods
    void start() override;
    void cleanup() override;
    void send_key(int vk) override;

  private:
    Mapper &mapper;
};
