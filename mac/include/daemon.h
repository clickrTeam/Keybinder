#pragma once
#include "abstract_daemon.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>

class Daemon : public AbstractDaemon {
  public:
    // Constructor and Destructor
    Daemon();
    ~Daemon();

    // Override abstract class methods
    void start() override;
    void cleanup() override;
    void send_key(int vk) override;

  private:
    IOHIDManagerRef hidManager;

    // Callback for input events
    static void input_event_callback(void *context, IOReturn result,
                                     void *sender, IOHIDValueRef value);

    // Callback for matching devices
    static void device_matching_callback(void *context, IOReturn result,
                                         void *sender, IOHIDDeviceRef device);
};
