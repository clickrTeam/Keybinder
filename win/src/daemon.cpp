#include "daemon.h"
#include <QCoreApplication>
#include "mapper.h"
#include "daemon.h"
#include <windows.h>
#include <winuser.h>

HHOOK kbd = NULL; // Global hook handle
// TODO: fill this in
Daemon::Daemon(Mapper &m) : mapper(m) {
    qDebug() << "Daemon created";
    qDebug() << "Starting Win systems";
    kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &Daemon::KeyboardHook, 0, 0);
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
    qDebug() << "press";
    // Inject Shift+W manually
    INPUT inputs[2] = {};

    // Press x
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vk;

    // Release x
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vk;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
    qDebug() << "Key sent";
}

LRESULT CALLBACK Daemon::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // Ignore system call - I believe if nCode < 0 then its a system call and we should always ignore it.
    if (nCode < 0) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    // nCode - N/I, wParam tells us what type of event happend, lParam is the key and scan code and flags
    KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam); // reinterpret_cast is c++ style casting instead of c style (KBDLLHOOKSTRUCT)lParam
    // lParam has vkCode which is the virtual key code while scanCode is the hardware key code which can be diffrent for the same keys

           // Ignore sythesized Inputs
    if (kbdStruct->flags & LLKHF_INJECTED) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    // Handle input
    switch (wParam)
    {
    case WM_KEYDOWN: case WM_SYSKEYDOWN: {
        InputEvent e;
        e.keycode = kbdStruct->vkCode;
        e.type = KeyEventType::Press;
            if (mapper.mapInput(e))
            return 1; // Suppress keypress
        break;
    }
    case WM_KEYUP: {
        break;
    }
    default: {
        break;
    }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
    // continue down the chain of hooks
    // return CallNextHookEx(NULL, nCode, wParam, lParam);
}

