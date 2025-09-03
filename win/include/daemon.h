#pragma once
#include "abstract_daemon.h"
#include "mapper.h"
#include <QDebug>
#include <qglobalstatic.h>
#include <windows.h>

class Daemon : public AbstractDaemon {
  public:
    // Constructor and Destructor
    Daemon(KeySender);
    ~Daemon();

    // Override abstract class methods
    void start() override;
    void cleanup() override;
    void send_keys(const QList<InputEvent> &vk) override;
    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
};
