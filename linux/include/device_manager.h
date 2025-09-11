#pragma once
#include <QString>
#include <linux/uinput.h>
#include <libevdev-1.0/libevdev/libevdev.h>

///
/// \brief Determines whether a user has a eventX device configured as their
/// keyboard in the config file
/// \param config_file_path: The path to the config file
/// \return QString eventX if an eventX device is found, empty string otherwise
///
QString retrieve_eventX(QString config_file_path);

///
/// \brief Determines whether a user has a eventX device configured as their
/// keyboard in the config file. This uses the default file path
/// \return QString eventX if an eventX device is found, empty string otherwise
///
QString retrieve_eventX();

///
/// \brief Stores the keyboard's eventX file in the config file
/// \param eventX_path: The string that represents which event device
/// corresponds to the keyboard
/// \return True if config file has successfully written, false otherwise
///
bool record_eventX(QString eventX_path);

///
/// \brief Detect the system keyboard device node (/dev/input/eventX).
///
/// This function uses systemd's sd-device API to enumerate input devices
/// and filter for those that udev has already marked as keyboards
/// (ID_INPUT_KEYBOARD=1). If multiple candidate devices are found, it
/// further refines the choice by checking with libevdev whether the
/// device has the expected set of alphanumeric + modifier keys.
///
/// If no strong match is found, the function falls back to a slower
/// interactive detection routine (`detect_keyboard_fallback()`), where
/// the user can press the spacebar to identify the correct device.
///
/// \return QString Path to the detected keyboard device (e.g. "/dev/input/event3"),
///                or an empty QString if none found.
///
QString detect_keyboard();

///
/// \brief Prompts the user to press a key on their keyboard, then records which
/// device that press came from. Times out after so long to prevent users from
/// getting stuck.
/// \return The path to the eventX file corresponding to the keyboard.
///
QString detect_keyboard_fallback();

///
/// \brief Check if an initialized libevdev device looks like a "full" keyboard.
///
/// Many devices expose EV_KEY events (e.g. laptop hotkey panels, remote controls,
/// volume/mute buttons), but they are not full keyboards. This function applies
/// a heuristic: it checks that the device supports a set of representative keys
/// that only a real alphanumeric keyboard would have.
///
/// Keys checked:
///  - KEY_A and KEY_Z  → ensures alphabetic range is present
///  - KEY_SPACE        → ensures spacebar exists
///  - KEY_ENTER        → ensures return/enter key exists
///  - KEY_LEFTSHIFT    → ensures at least one modifier is present
///
/// \param dev Pointer to a libevdev device (already initialized).
/// \return true if the device appears to be a full keyboard, false otherwise.
///
bool is_full_keyboard(libevdev *dev);

///
/// \brief Set up a virtual keyboard device using the Linux uinput subsystem.
///
/// \return A file descriptor of the virtual keyboard device.
/// 
/// This function opens /dev/uinput and registers a new virtual input device
/// that behaves like a keyboard. It enables support for key events (EV_KEY)
/// and synchronization events (EV_SYN), then marks all possible key codes
/// (0..KEY_MAX) as supported. The device is given a USB-like identity and the
/// name "clickr_virtual_keyboard", and is created in the kernel
/// so it appears as an input device (e.g., /dev/input/eventX).
///
/// On success, the file descriptor for the created uinput device is returned. 
/// On failure, the function prints an error and returns -1.
int setup_uinput_device();