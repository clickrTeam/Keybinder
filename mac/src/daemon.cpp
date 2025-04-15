#include "daemon.h"
#include "event.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <MacTypes.h>
#include <iostream>
#include <qdebug.h>

// Heavily based on https://github.com/psych3r/driverkit
Daemon::Daemon(Mapper &m)
    : mapper(m), matching_dictionary(nullptr),
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
    client = std::make_shared<
        pqrs::karabiner::driverkit::virtual_hid_device_service::client>();
    auto copy = client;

    client->connected.connect([copy] {
        qDebug() << "connected to karabiner driver";
        pqrs::karabiner::driverkit::virtual_hid_device_service::
            virtual_hid_keyboard_parameters parameters;
        parameters.set_country_code(pqrs::hid::country_code::us);
        copy->async_virtual_hid_keyboard_initialize(parameters);
    });

    client->connect_failed.connect([](auto &&error_code) {
        std::cout << "connect failed: " << error_code;
        exit(1);
    });

    client->closed.connect(
        [] { qDebug() << "connection to karabiner driver closed"; });

    client->error_occurred.connect([](auto &&error_code) {
        qDebug() << "error occurred: " << error_code;
        // TODO: should be exit here?
        exit(1);
    });

    client->driver_activated.connect([](auto &&driver_activated) {
        static std::optional<bool> previous_value;
        if (previous_value != driver_activated) {
            qDebug() << "driver activated: " << driver_activated;
            previous_value = driver_activated;
        }
    });

    client->driver_connected.connect([](auto &&driver_connected) {
        static std::optional<bool> previous_value;
        if (previous_value != driver_connected) {
            qDebug() << "driver connected: " << driver_connected;
            previous_value = driver_connected;
        }
    });

    client->driver_version_mismatched.connect(
        [](auto &&driver_version_mismatched) {
            static std::optional<bool> previous_value;
            if (previous_value != driver_version_mismatched) {
                qDebug() << "driver_version_mismatched "
                         << driver_version_mismatched << std::endl;
                previous_value = driver_version_mismatched;
            }
        });

    client->async_start();
}

Daemon::~Daemon() { cleanup(); }

void Daemon::start() {
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
            qDebug() << "Error opening device";
            exit(1);
        }
        IOHIDDeviceScheduleWithRunLoop(dev, CFRunLoopGetCurrent(),
                                       kCFRunLoopDefaultMode);
    }

    IOObjectRelease(iter);
    CFRunLoopRun();
}

void Daemon::cleanup() { std::cout << "Daemon cleaned up." << std::endl; }

void Daemon::send_key(int vk) {

    pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::
        keyboard_input report;
    report.keys.insert(vk);

    client->async_post_report(report);
}

void Daemon::handle_input_event(uint64_t value, uint32_t page, uint32_t code) {
    std::cout << "Key " << code << (value ? " pressed" : " released")
              << std::endl;

    mapper.mapInput(InputEvent{
        .keycode = static_cast<int>(code),
        .type = value ? KeyEventType::Press : KeyEventType::Relase,
    });
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
