#include "daemon.h"
#include "thread"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <MacTypes.h>
#include <QTimer>
#include <iostream>
#include <qdebug.h>

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

    qDebug() << "Daemon created";
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
    std::cout << "send_key called with key code: " << vk << std::endl;
    // TODO: hook up with driver
}

void Daemon::input_event_callback(void *context, IOReturn result, void *sender,
                                  IOHIDValueRef value) {
    Daemon *self = reinterpret_cast<Daemon *>(context);
    // TODO: use self to call some non static method
    IOHIDElementRef element = IOHIDValueGetElement(value);
    if (!element)
        return;

    uint32_t usagePage = IOHIDElementGetUsagePage(element);
    uint32_t usage = IOHIDElementGetUsage(element);

    // Process only keyboard events (usage page 0x07)
    if (usagePage == 0x07) {
        int pressed = IOHIDValueGetIntegerValue(value);
        std::cout << "Key " << usage << (pressed ? " pressed" : " released")
                  << std::endl;
    }
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
