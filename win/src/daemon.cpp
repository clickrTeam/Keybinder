#include "daemon.h"
#include <QCoreApplication>
#include <windows.h>
#include <winuser.h>

// TODO: fill this in
Daemon::Daemon() {
    qDebug() << "Daemon created";
    qDebug() << "Starting Win systems";
    kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &KeyboardHook, 0, 0);
    if (!kbd) {
        qCritical() << "Failed to install keyboard hook!";
        return;
    }
   // idHook, HookProc, Hinstance - N/I, dwThreadId - N/I
   // hooks idHook to HookProc, this happens before the os processes the input
}

Daemon::~Daemon() { qDebug() << "Daemon destroyed"; }

void Daemon::start() {
    qDebug() << "Daemon started";
    qDebug() << "Win, message pump starting";
    // message loop - spin until the user presses a key, somehow a common
    // practice in windows programming. aka message pump
    MSG msg;
    while (GetMessage(&msg, NULL, NULL, NULL) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Daemon::cleanup();
}

void Daemon::cleanup() {
    if (kbd) {
        UnhookWindowsHookEx(kbd);
        kbd = NULL;
        qDebug() << "Keyboard hook uninstalled.";
    }
    qDebug() << "Daemon cleaned up";
}

void Daemon::send_key(int vk) {
    qDebug() << "Key sent";
}

