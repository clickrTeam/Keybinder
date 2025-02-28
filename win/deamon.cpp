#include <QDebug>
#include <QCoreApplication>
#include "deamon.h"
#include <windows.h>
#include <winuser.h>

HHOOK kbd = NULL; // Global hook handle
Profile activeProfile;
Layer activeLayer;

void winStartDeamon(Profile _activeProfile) {
    qDebug() << "Starting Win systems";
    kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &KeyboardHook, 0, 0);
    if (!kbd) {
        qDebug() << "Failed to install keyboard hook!";
        return;
    }
    activeProfile = _activeProfile;
    activeLayer = activeProfile.layers[0];
    qDebug() << activeLayer.tapKeyBinds.keys();

    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, []() {
        cleanup();
    });
    winDeamon();

    // idHook, HookProc, Hinstance - N/I, dwThreadId - N/I
    // hooks idHook to HookProc, this happens before the os processes the input
};

void winDeamon() {
    qDebug() << "Win, message pump starting";
    // message loop - spin until the user presses a key, somehow a common practice in windows programming. aka message pump
    MSG msg;
    while (GetMessage(&msg, NULL, NULL, NULL) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    cleanup();
}

void cleanup() {
    if (kbd) {
        UnhookWindowsHookEx(kbd);
        kbd = NULL;
        qDebug() << "Keyboard hook uninstalled.";
    }
}

//  - N/I = Not Important
// WM - Windows Message
// SOURCE- https://youtu.be/QIWw0jZqGKA?si=snqwlN0HlzcFlaWn
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
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
    case WM_KEYDOWN: {
        char _c = MapVirtualKey(kbdStruct->vkCode, MAPVK_VK_TO_CHAR);
        QString c = QString(_c);
        qDebug() << GetKeyName(kbdStruct->vkCode);
        KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        qDebug() << c;

        if (activeLayer.tapKeyBinds.contains(c)) {
            qDebug() << "Win";
        } else {
            qDebug() << "poopoo";
        }
        // if (kbdStruct->vkCode == 'W') // Detect 'W' keydown
        // {
        //     // Inject Shift+W manually
        //     INPUT inputs[4] = {};

        //     // Press Shift
        //     inputs[0].type = INPUT_KEYBOARD;
        //     inputs[0].ki.wVk = VK_SHIFT;

        //     // Press W
        //     inputs[1].type = INPUT_KEYBOARD;
        //     inputs[1].ki.wVk = 'W';

        //     // Release W
        //     inputs[2].type = INPUT_KEYBOARD;
        //     inputs[2].ki.wVk = 'W';
        //     inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

        //     // Release Shift
        //     inputs[3].type = INPUT_KEYBOARD;
        //     inputs[3].ki.wVk = VK_SHIFT;
        //     inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

        //     SendInput(4, inputs, sizeof(INPUT));

        //     return 1; // Suppress the original W key release
        // }
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

std::string GetKeyName(unsigned int virtualKey)
{
    unsigned int scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    switch (virtualKey)
    {
    case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
    case VK_PRIOR: case VK_NEXT: // page up and page down
    case VK_END: case VK_HOME:
    case VK_INSERT: case VK_DELETE:
    case VK_DIVIDE: // numpad slash
    case VK_NUMLOCK:
    {
        scanCode |= 0x100; // set extended bit
        break;
    }
    }
    char keyName[50];
    if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName)) != 0)
    {
        return keyName;
    }
    else
    {
        return "[Error]";
    }
}
