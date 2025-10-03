#pragma once
#include "event.h"
#include <QList>

#include "event.h"
class AbstractDaemon {
  public:
    virtual ~AbstractDaemon() = default;

    // This should take some mapper object or just a key channel
    virtual void start() = 0;
    virtual void cleanup() = 0;
    // Should take some kind of key event
    virtual void send_outputs(const QList<OutputEvent> &vk) = 0;
};
