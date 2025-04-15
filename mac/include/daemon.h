#pragma once
#include "abstract_daemon.h"
#include "mapper.h"
#include <AvailabilityMacros.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <filesystem>
#include <mach/mach_error.h>
#include <unistd.h>

#include "virtual_hid_device_driver.hpp"
#include "virtual_hid_device_service.hpp"

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
    CFMutableDictionaryRef matching_dictionary;
    IONotificationPortRef notification_port;

    // Helper: create a matching dictionary for keyboards
    CFDictionaryRef create_keyboard_matching_dictionary();

    void handle_input_event(uint64_t value, uint32_t page, uint32_t code);

    // Callback for input events (called when an input value changes)
    static void input_event_callback(void *context, IOReturn result,
                                     void *sender, IOHIDValueRef value);

    // Callback for when a matching device is found
    static void device_matching_callback(void *context, IOReturn result,
                                         void *sender, IOHIDDeviceRef device);
    // void start_monitoring_loop();

    // Helper methods
    static CFStringRef get_property(mach_port_t item, const char *property);
    static CFStringRef from_cstr(const char *str);
};
