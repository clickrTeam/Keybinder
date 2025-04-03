#pragma once
class AbstractDaemon {
  public:
    // TODO add either a key sender or a mapper object here
    virtual void start_daemon() = 0;
    virtual void cleanup() = 0;
    // TODO: Add common key-code enum here and possibly some modifiers
    // (key up key down etc)
    virtual void send_key() = 0;
};
