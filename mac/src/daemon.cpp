#include "daemon.h"
#include "event.h"
#include "key_code.h"
#include "key_map.h"
#include "script_runner.h"
#include "util.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <MacTypes.h>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QTemporaryFile>
#include <cstdint>
#include <iostream>
#include <spawn.h>
#include <sys/stat.h>
#include <unistd.h>

// Heavily based on https://github.com/psych3r/driverkit
Daemon::Daemon(KeySender key_sender)
    : key_sender(key_sender), app_focus_listener([&key_sender](auto app_name) {
          key_sender.send_key(AppFocusedEvent{app_name});
      }),
      matching_dictionary(nullptr),
      notification_port(IONotificationPortCreate(kIOMainPortDefault)) {
    matching_dictionary = IOServiceMatching(kIOHIDDeviceKey);
    UInt32 generic_desktop = kHIDPage_GenericDesktop;
    UInt32 gd_keyboard = kHIDUsage_GD_Keyboard;
    CFNumberRef page_number = CFNumberCreate(
        kCFAllocatorDefault, kCFNumberSInt32Type, &generic_desktop);
    CFNumberRef usage_number =
        CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &gd_keyboard);
    CFDictionarySetValue(matching_dictionary, CFSTR(kIOHIDDeviceUsagePageKey),
                         page_number);
    CFDictionarySetValue(matching_dictionary, CFSTR(kIOHIDDeviceUsageKey),
                         usage_number);
    CFRelease(page_number);
    CFRelease(usage_number);

    // Seting up driver client
    // pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::
    //     keyboard_input report;
    // client->error_occurred.connect([](auto &&error_code) {
    //     qDebug() << "error occurred: " << error_code;
    //     // TODO: should be exit here?
    //     exit(1);
    // });

    // client->driver_activated.connect([](auto &&driver_activated) {
    //     static std::optional<bool> previous_value;
    //     if (previous_value != driver_activated) {
    //         qDebug() << "driver activated: " << driver_activated;
    //         previous_value = driver_activated;
    //     }
    // });
    //
    // client->driver_connected.connect([](auto &&driver_connected) {
    //     static std::optional<bool> previous_value;
    //     if (previous_value != driver_connected) {
    //         qDebug() << "driver connected: " << driver_connected;
    //         previous_value = driver_connected;
    //     }
    // });
    // client->driver_version_mismatched.connect(
    //     [](auto &&driver_version_mismatched) {
    //         static std::optional<bool> previous_value;
    //         if (previous_value != driver_version_mismatched) {
    //             qDebug() << "driver_version_mismatched "
    //                      << driver_version_mismatched << std::endl;
    //             previous_value = driver_version_mismatched;
    //         }
    //     });
}

Daemon::~Daemon() { cleanup(); }

void Daemon::start() {
    app_focus_listener.start();
    io_iterator_t iter = IO_OBJECT_NULL;
    CFRetain(matching_dictionary);
    IOServiceGetMatchingServices(kIOMainPortDefault, matching_dictionary,
                                 &iter);
    for (mach_port_t device = IOIteratorNext(iter); device;
         device = IOIteratorNext(iter)) {
        CFStringRef karabiner = from_cstr(
            "Karabiner"); // Karabiner DriverKit VirtualHIDKeyboard 1.7.0
        CFStringRef device_key = get_property(device, kIOHIDProductKey);
        if (!device_key ||
            CFStringFind(karabiner, device_key, kCFCompareCaseInsensitive)
                    .location != kCFNotFound) {
            continue;
        }

        IOHIDDeviceRef dev = IOHIDDeviceCreate(kCFAllocatorDefault, device);
        // source_devices[device] = dev;
        IOHIDDeviceRegisterInputValueCallback(dev, input_event_callback, this);
        kern_return_t kr = IOHIDDeviceOpen(dev, kIOHIDOptionsTypeSeizeDevice);
        if (kr != kIOReturnSuccess) {
            qDebug() << "Error opening device: " << mach_error_string(kr)
                     << "dev: " << device_key;
        }
        IOHIDDeviceScheduleWithRunLoop(dev, CFRunLoopGetCurrent(),
                                       kCFRunLoopDefaultMode);
    }

    qDebug() << "Making karabiner client";
    pqrs::dispatcher::extra::initialize_shared_dispatcher();
    client = std::make_shared<
        pqrs::karabiner::driverkit::virtual_hid_device_service::client>();

    client->connected.connect([this] {
        std::cout << "connected" << std::endl;

        pqrs::karabiner::driverkit::virtual_hid_device_service::
            virtual_hid_keyboard_parameters parameters;
        parameters.set_country_code(pqrs::hid::country_code::us);

        client->async_virtual_hid_keyboard_initialize(parameters);
    });

    client->connect_failed.connect([](auto &&error_code) {
        std::cout << "connect failed: " << error_code;
        exit(1);
    });

    client->virtual_hid_keyboard_ready.connect([](auto &&ready) {
        static std::optional<bool> previous_value;

        if (previous_value != ready) {
            qDebug() << "virtual HID keyboard is ready";
            previous_value = ready;
        }
    });

    client->closed.connect(
        [] { qDebug() << "connection to karabiner driver closed"; });

    client->async_start();

    IOObjectRelease(iter);
    CFRunLoopRun();
}

/// TODO: fill this in not exactly sure how to unsieze devices
void Daemon::cleanup() {
    app_focus_listener.stop();
    std::cout << "Daemon cleaned up." << std::endl;
}

void Daemon::send_outputs(const QList<OutputEvent> &events) {
    for (const OutputEvent &event : events) {
        std::visit(overloaded{
                       [&](const KeyEvent &key) {
                           qDebug() << "SENDING KEY "
                                    << str_to_keycode.find_backward(key.keycode)
                                    << (key.type == KeyEventType::Press
                                            ? " pressed"
                                            : " released");

                           uint16_t raw_keycode =
                               int_to_keycode.find_backward(key.keycode);
                           if (key.type == KeyEventType::Press)
                               report.keys.insert(raw_keycode);
                           else if (key.type == KeyEventType::Release)
                               report.keys.erase(raw_keycode);

                           client->async_post_report(report);
                       },
                       [&](const RunScript &script) {
                           run_script(script.interpreter, script.script);
                       },
                   },
                   event);
    }
}

void Daemon::handle_input_event(uint64_t value, uint32_t page, uint32_t code) {
    // TODO: be more sure about what to filter out
    if (code > 1000 | code < 2)
        return;

    std::cout << "Key " << code << (value ? " pressed" : " released")
              << std::endl;
    if (!int_to_keycode.contains_forward(code))
        return;

    auto event = KeyEvent{
        .keycode = int_to_keycode.find_forward(code),
        .type = value ? KeyEventType::Press : KeyEventType::Release,
    };

    if (!key_sender.send_key(event)) {
        send_outputs({event});
    }
}
void Daemon::input_event_callback(void *context, IOReturn result, void *sender,
                                  IOHIDValueRef value) {
    Daemon *self = reinterpret_cast<Daemon *>(context);
    // TODO: use self to call some non static method
    IOHIDElementRef element = IOHIDValueGetElement(value);
    if (!element)
        return;
    self->handle_input_event(IOHIDValueGetIntegerValue(value),
                             IOHIDElementGetUsagePage(element),
                             IOHIDElementGetUsage(element));
}

// TODO: not used currently but should be used in the future to allow for users
// to connect keyboards to the system without a restart
void Daemon::device_matching_callback(void *context, IOReturn result,
                                      void *sender, IOHIDDeviceRef device) {
    Daemon *self = reinterpret_cast<Daemon *>(context);
    if (!self)
        return;

    // This for some reason does not actaully sieze the device
    IOReturn ret = IOHIDDeviceOpen(device, kIOHIDOptionsTypeSeizeDevice);
    if (ret != kIOReturnSuccess) {
        qDebug() << "Failed to open device with seize flag.";
        exit(1);
    }

    // Register our input callback on this device.
    IOHIDDeviceRegisterInputValueCallback(device, Daemon::input_event_callback,
                                          self);

    // Schedule the device on the current run loop.
    IOHIDDeviceScheduleWithRunLoop(device, CFRunLoopGetCurrent(),
                                   kCFRunLoopDefaultMode);
}

CFStringRef Daemon::get_property(mach_port_t item, const char *property) {
    return (CFStringRef)IORegistryEntryCreateCFProperty(
        item, from_cstr(property), kCFAllocatorDefault, kIOHIDOptionsTypeNone);
}

CFStringRef Daemon::from_cstr(const char *str) {
    return CFStringCreateWithCString(kCFAllocatorDefault, str,
                                     CFStringGetSystemEncoding());
}
