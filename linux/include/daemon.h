#pragma once

#include "abstract_daemon.h"
#include "profile.h"
#include <linux/uinput.h>  // Required for injecting events
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <QDebug>
using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::vector;
using std::map;

class Daemon : public AbstractDaemon {
  private:
    int keyb_fd = -1;
    int uinput_fd = -1;
    struct libevdev *keyb;
    QString event_keyb_path = "";
  public:
    // Constructor and Destructor
    Daemon();
    ~Daemon();

    // Override abstract class methods
    void start() override;

    ///
    /// \brief Closes all fds and cleans up the deamon in order for the keyboard to
    /// return to normal function.
    ///
    void cleanup() override;
    void send_key(int vk) override;

    ///
    /// \brief Starts the deamon which allows for key presses to be intercepted
    /// \param activeProfile: The profile which will be loaded into the deamon
    ///
    void linux_start_deamon(Profile activeProfile);

    ///
    /// \brief Opens the uinput device to send key events.
    ///
    void setup_uinput_device();
};
