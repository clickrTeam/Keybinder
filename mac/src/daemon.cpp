#include "daemon.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <MacTypes.h>
#include <QTimer>
#include <iostream>
#include <qdebug.h>

// Constructor: initialize member variables.
Daemon::Daemon()
    : matching_dictionary(nullptr),
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
}

// Destructor: clean up resources.
Daemon::~Daemon() { cleanup(); }

void Daemon::start() {
    io_iterator_t iter = IO_OBJECT_NULL;
    CFRetain(matching_dictionary);
    IOServiceGetMatchingServices(kIOMainPortDefault, matching_dictionary,
                                 &iter);
    for (mach_port_t curr = IOIteratorNext(iter); curr;
         curr = IOIteratorNext(iter)) {
    }

    IOObjectRelease(iter);

    // Build a matching dictionary for keyboards:
    //   Generic Desktop (0x01) and Keyboard (0x06)
    int page = 0x01;
    int usage = 0x06;
    CFNumberRef pageNumber =
        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
    CFNumberRef usageNumber =
        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
    const void *keys[] = {CFSTR(kIOHIDDeviceUsagePageKey),
                          CFSTR(kIOHIDDeviceUsageKey)};
    const void *values[] = {pageNumber, usageNumber};
    CFDictionaryRef matchingDict = CFDictionaryCreate(
        kCFAllocatorDefault, keys, values, 2, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    CFRelease(pageNumber);
    CFRelease(usageNumber);

    // Set the matching criteria.
    IOHIDManagerSetDeviceMatching(hidManager, matchingDict);
    CFRelease(matchingDict);

    // Register a device-matching callback.
    // For each device found, our staticDeviceMatchingCallback will be invoked.
    IOHIDManagerRegisterDeviceMatchingCallback(
        hidManager, Daemon::device_matching_callback, this);

    // Schedule the HID manager on the current run loop.
    IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(),
                                    kCFRunLoopDefaultMode);

    // Open the HID manager.
    IOReturn ret = IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);
    if (ret != kIOReturnSuccess) {
        std::cerr << "Error: Unable to open IOHIDManager (error: 0x" << std::hex
                  << ret << ")" << std::endl;
        return;
    }

    std::cout << "Daemon started: Keyboard devices are seized and key events "
                 "will be captured."
              << std::endl;

    CFRunLoopRun();
}

void Daemon::cleanup() {
    if (hidManager) {
        IOHIDManagerUnscheduleFromRunLoop(hidManager, CFRunLoopGetCurrent(),
                                          kCFRunLoopDefaultMode);
        IOHIDManagerClose(hidManager, kIOHIDOptionsTypeNone);
        CFRelease(hidManager);
        hidManager = nullptr;
    }
    std::cout << "Daemon cleaned up." << std::endl;
}

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
