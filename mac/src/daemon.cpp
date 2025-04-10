#include "daemon.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>
#include <iostream>
#include <qdebug.h>

// Constructor: initialize member variables.
Daemon::Daemon() : hidManager(nullptr) {}

// Destructor: clean up resources.
Daemon::~Daemon() { cleanup(); }

// start():
//   - Creates the HID Manager.
//   - Sets a matching dictionary for keyboards (Generic Desktop: usage page
//   0x01, usage: 0x06).
//   - Registers a device-matching callback (to seize each keyboard).
//   - Opens the HID Manager and starts the run loop.
void Daemon::start() {
    // Create the HID manager.
    hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (!hidManager) {
        std::cerr << "Error: Unable to create IOHIDManager" << std::endl;
        return;
    }

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

    // Run the CFRunLoop to process events (this call blocks indefinitely).
    CFRunLoopRun();
}

// cleanup():
//   Unschedule and close the HID manager, then release resources.
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

// send_key():
//   In this basic example, we simply log the key code that would be sent.
void Daemon::send_key(int vk) {
    std::cout << "send_key called with key code: " << vk << std::endl;
    // TODO: hook up with driver
}

void Daemon::input_event_callback(void *context, IOReturn result, void *sender,
                                  IOHIDValueRef value) {
    Daemon *self = reinterpret_cast<Daemon *>(context);
    //  TODO use self to call some non static method
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

    // Attempt to open the device with the 'seize' flag so that its key events
    // are not passed along.
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
