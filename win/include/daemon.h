#pragma once
#include <QDebug>
#include <qglobalstatic.h>
#include <windows.h>
#include "abstract_daemon.h"
#include "mapper.h"

class Daemon : public AbstractDaemon {
  public:
    // Constructor and Destructor
    Daemon(Mapper &m);
    ~Daemon();

    // Override abstract class methods
    void start() override;
    void cleanup() override;
    void send_key(const QList<InputEvent>& vk) override;
    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
};

