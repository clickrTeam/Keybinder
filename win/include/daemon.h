#pragma once
#include <QDebug>
#include <qglobalstatic.h>
#include <windows.h>
#include "abstract_daemon.h"
class Daemon : public AbstractDaemon {
  public:
    // Constructor and Destructor
    Daemon();
    ~Daemon();

    // Override abstract class methods
    void start() override;
    void cleanup() override;
    void send_key(int vk) override;

    HHOOK kbd = NULL; // Global hook handle
};
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam);

