#pragma once

#include "abstract_daemon.h"
#include "key_channel.h"
#include <QDebug>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/uinput.h> // Required for injecting events
#include <map>
#include <unistd.h>
#include <vector>
using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

class Daemon : public AbstractDaemon {
  private:
    int keyb_fd = -1;
    int uinput_fd = -1;
    struct libevdev *keyb = nullptr;
    QString event_keyb_path = "";
    bool is_running = false;
    KeySender key_sender;

    void send_keys_helper(const QList<InputEvent> &vk, int fd);
    void send_key(int key_code, int state, int fd);

  public:
    // Constructor and Destructor
    Daemon(KeySender);
    ~Daemon();

    // Override abstract class methods
    void start() override;

    ///
    /// \brief Closes all fds and cleans up the deamon in order for the keyboard
    /// to return to normal function.
    ///
    void cleanup() override;
    
    ///
    /// \brief Sends a sequence of key press and release events through the
    /// uinput device.
    ///
    /// This function iterates over a list of input events, translates each into
    /// a Linux input_event structure, and writes it to the uinput file
    /// descriptor. For each key press/release, a synchronization (EV_SYN /
    /// SYN_REPORT) event is also sent to mark the end of that event frame.
    ///
    /// \param vk A QList of InputEvent objects, each representing a keycode and
    ///           whether the key is pressed or released.
    ///
    /// \param fd A uinput file descriptor of a device to send the keys to
    ///
    /// Behavior details:
    /// - For EV_KEY events:
    ///     - value = 1 → key press
    ///     - value = 0 → key release
    /// - For EV_SYN events:
    ///     - value is always set to 0 (SYN_REPORT convention).
    ///
    /// Example:
    ///   Passing a list with one "press A" and one "release A" event will
    ///   generate the corresponding key press and release in the virtual
    ///   input device.
    ///
    void send_keys(const QList<InputEvent> &vk) override;
};
