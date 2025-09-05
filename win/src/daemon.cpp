#include "daemon.h"
#include "event.h"
#include "key_channel.h"
#include "key_map.h"
#include <QCoreApplication>
#include <windows.h>
#include <winuser.h>

HHOOK kbd = NULL; // Global hook handle
KeySender key_sender(nullptr);
const ULONG_PTR InfoIdentifier =
    0x1234ABCD; // allowed collisions otherwise,
                // ((ULONG_PTR)GetCurrentProcessId() << 32) | 0x1234ABCD

Daemon::Daemon(KeySender key_sender_tmp) {
    key_sender = key_sender_tmp;
    qDebug() << "Daemon created";
    qDebug() << "Starting Win systems";
    kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &Daemon::HookProc, 0, 0);
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

void Daemon::send_keys(const QList<InputEvent> &vk) {
    qDebug() << "Sending" << vk.count() << "keys";

    QVector<INPUT> inputs;
    inputs.resize(vk.count() * 2); // Press + Release for each key

    for (int i = 0; i < vk.count(); ++i) {
        InputEvent v = vk[i];

        // Press
        inputs[i].type = INPUT_KEYBOARD;
        inputs[i].ki.wVk = int_to_keycode.find_backward(v.keycode);
        // identify key so we can ignore it.
        inputs[i].ki.dwExtraInfo = InfoIdentifier;
        if (v.type == KeyEventType::Press) {
            inputs[i].ki.dwFlags = 0;
        } else {
            inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
        }
    }

    UINT sent = SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
    if (sent != inputs.size()) {
        qWarning() << "SendInput failed. Sent" << sent << "of" << inputs.size();
    } else {
        qDebug() << "Keys sent!";
    }
}

LRESULT CALLBACK Daemon::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // Ignore system call - I believe if nCode < 0 then its a system call and we
    // should always ignore it.
    if (nCode < 0) {
        // qDebug() << "Key ignored from system."; // - useful to debug what
        // goes through
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    // nCode - N/I, wParam tells us what type of event happend, lParam is the
    // key and scan code and flags
    KBDLLHOOKSTRUCT *kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT *>(
        lParam); // reinterpret_cast is c++ style casting instead of c style
                 // (KBDLLHOOKSTRUCT)lParam
    // lParam has vkCode which is the virtual key code while scanCode is the
    // hardware key code which can be diffrent for the same keys

    // Ignore sythesized Inputs
    if (kbdStruct->flags & LLKHF_INJECTED &&
        kbdStruct->dwExtraInfo == InfoIdentifier) {
        // qDebug() << "Key ignored from injection."; // - useful to debug what
        // goes through
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    // FIXME: this is not a real solution
    if (!int_to_keycode.contains_forward(kbdStruct->vkCode))
        return 0;

    // Handle input
    switch (wParam) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        InputEvent e;
        e.keycode = int_to_keycode.find_forward(kbdStruct->vkCode);
        e.type = KeyEventType::Press;
        if (key_sender.send_key(e))
            return 1; // Suppress keypress
        break;
    }
    case WM_KEYUP: {
        InputEvent e;
        e.keycode = int_to_keycode.find_forward(kbdStruct->vkCode);
        e.type = KeyEventType::Release;
        if (key_sender.send_key(e))
            return 1; // Suppress keypress
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
