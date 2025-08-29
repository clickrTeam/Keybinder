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
    struct libevdev *keyb;
    QString event_keyb_path = "";
    bool is_running = false;
    KeySender key_sender;

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
    void send_keys(const QList<InputEvent> &vk) override;

    ///
    /// \brief Opens the uinput device to send key events.
    ///
    void setup_uinput_device();
};
