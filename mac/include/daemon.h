#include "abstract_daemon.h"
class Daemon : public AbstractDaemon {
  public:
    // Constructor and Destructor
    Daemon();
    ~Daemon();

    // Override abstract class methods
    void start_daemon() override;
    void cleanup() override;
    void send_key() override;
};
