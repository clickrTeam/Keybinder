#pragma once
#include "abstract_daemon.h"
#include "key_channel.h"
#include "mapper.h"
#include <AvailabilityMacros.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <mach/mach_error.h>
#include <memory>
#include <unistd.h>

// Has to come before the virtual_hid_device_* imports
#include <filesystem>

#include "virtual_hid_device_driver.hpp"
#include "virtual_hid_device_service.hpp"

class Daemon : public AbstractDaemon {
  public:
    // Constructor and Destructor
    Daemon(KeySender);
    ~Daemon();

    // Override abstract class methods
    void start() override;
    void cleanup() override;
    void send_keys(const QList<InputEvent> &events) override;

  private:
    KeySender key_sender;
    CFMutableDictionaryRef matching_dictionary;
    IONotificationPortRef notification_port;
    std::shared_ptr<
        pqrs::karabiner::driverkit::virtual_hid_device_service::client>
        client;

    pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::
        keyboard_input report;

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
