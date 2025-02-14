#include <QDebug>
#include "deamon.h"
#include <windows.h>
#include <winuser.h>

void startup() {
    qDebug() << "Started";
    HHOOK kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &KeyboardHook, 0, 0);
    winDeamon(&kbd);

    // idHook, HookProc, Hinstance - N/I, dwThreadId - N/I
    // hooks idHook to HookProc, this happens before the os processes the input
};

void winDeamon(HHOOK* kbd) {
    // message loop - spin until the user presses a key, somehow a common practice in windows programming. aka message pump
    MSG msg;
    while (GetMessage(&msg, NULL, NULL, NULL) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(*kbd);
}

//  - N/I = Not Important
// WM - Windows Message
// SOURCE- https://youtu.be/QIWw0jZqGKA?si=snqwlN0HlzcFlaWn
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    // nCode - N/I, wParam tells us what type of event happend, lParam is the key and scan code and flags
    KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam); // reinterpret_cast is c++ style casting instead of c style (KBDLLHOOKSTRUCT)lParam
    // lParam has vkCode which is the virtual key code while scanCode is the hardware key code which can be diffrent for the same keys

    switch (wParam)
    {
    case WM_KEYDOWN:
        char c = MapVirtualKey(kbdStruct->vkCode, MAPVK_VK_TO_CHAR);
        qDebug() << GetKeyName(kbdStruct->vkCode);
        break;

        // default:
        //     return CallNextHookEx(NULL, nCode, wParam, lParam);
        //     break;
    }
    // continue down the chain of hooks
    return CallNextHookEx(NULL, nCode, wParam, lParam);
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
