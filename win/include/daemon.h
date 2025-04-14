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
    void send_key(int vk) override;
    static LRESULT CALLBACK Trampoline(int nCode, WPARAM wParam, LPARAM lParam) {
        KeyboardHook* instance = reinterpret_cast<KeyboardHook*>(GetWindowLongPtr(GetCurrentWindow(), GWLP_USERDATA));
        return instance->HookProc(nCode, wParam, lParam);
    }
    LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

    HHOOK kbd = NULL; // Global hook handle
  private:
    Mapper &mapper;
};

