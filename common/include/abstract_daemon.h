#pragma once
#include <QList>

class AbstractDaemon {
  public:
    virtual ~AbstractDaemon() = default;

    // This should take some mapper object or just a key channel
    virtual void start() = 0;
    virtual void cleanup() = 0;
    // Should take some kind of key event
    virtual void send_key(int vk) = 0;
    virtual void send_keys(QList<int> vk) = 0;
};
