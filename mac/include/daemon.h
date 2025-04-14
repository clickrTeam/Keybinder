#pragma once
#include "abstract_daemon.h"
#include <AvailabilityMacros.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <condition_variable>
#include <errno.h>
#include <filesystem> // Include this before virtual_hid_device_service.hpp to avoid compile error
#include <iostream>
#include <mach/mach_error.h>
#include <map>
#include <mutex>
#include <thread>
#include <unistd.h>

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
    CFMutableDictionaryRef matching_dictionary;
    IONotificationPortRef notification_port;

    // Helper: create a matching dictionary for keyboards
    CFDictionaryRef create_keyboard_matching_dictionary();

    // Callback for input events (called when an input value changes)
    static void input_event_callback(void *context, IOReturn result,
                                     void *sender, IOHIDValueRef value);

    // Callback for when a matching device is found
    static void device_matching_callback(void *context, IOReturn result,
                                         void *sender, IOHIDDeviceRef device);
};
