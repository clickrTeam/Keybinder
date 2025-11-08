#include "daemon.h"
#include "event.h"
#include "key_channel.h"
#include "key_map.h"
#include <QCoreApplication>
#include <QtCore/qlogging.h>
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
    
    // Install keyboard hook
    kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &Daemon::HookProc, 0, 0);
    if (!kbd) {
        qCritical() << "Failed to install keyboard hook!";
        return;
    }
    
    // Install shell hook for monitoring app launches
    HHOOK shell = SetWindowsHookEx(WH_SHELL, &Daemon::ShellProc, 0, 0);
    if (!shell) {
        qCritical() << "Failed to install shell hook!";
        // Continue anyway as this is not critical
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

LRESULT CALLBACK Daemon::ShellProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    if (wParam == HSHELL_WINDOWCREATED) {
        HWND hwnd = (HWND)lParam;
        wchar_t className[256];
        wchar_t windowTitle[256];
        
        GetClassName(hwnd, className, 256);
        GetWindowText(hwnd, windowTitle, 256);
        
        QString appName = QString::fromWCharArray(windowTitle);
        qDebug() << "App launched:" << appName;
        
        // Create app launch event
        InputEvent e;
        e.keycode = KeyCode::None; // We might want to add a special code for apps
        e.type = KeyEventType::AppLaunch;
        key_sender.send_key(e);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Daemon::launchApp(const QString &appName) {
    QString appPath = getExecutablePath(appName);
    QString appId = getAppUserModelId(appName);
    
    if (!appPath.isEmpty()) {
        // Try launching via ShellExecute first
        HINSTANCE result = ShellExecute(NULL, L"open", appPath.toStdWString().c_str(), 
                                      NULL, NULL, SW_SHOWNORMAL);
        if ((INT_PTR)result > 32) {
            qDebug() << "Successfully launched app via ShellExecute:" << appName;
            return;
        }
    }
    
    if (!appId.isEmpty()) {
        // Try launching via modern app activation
        HSTRING appIdHString;
        WindowsCreateString(appId.toStdWString().c_str(), appId.length(), &appIdHString);
        
        ComPtr<IApplicationActivationManager> activationManager;
        HRESULT hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER,
                                    IID_PPV_ARGS(&activationManager));
        
        if (SUCCEEDED(hr)) {
            DWORD newProcessId;
            hr = activationManager->ActivateApplication(appIdHString, nullptr, AO_NONE, &newProcessId);
            if (SUCCEEDED(hr)) {
                qDebug() << "Successfully launched modern app:" << appName;
                WindowsDeleteString(appIdHString);
                return;
            }
            WindowsDeleteString(appIdHString);
        }
    }
    
    qWarning() << "Failed to launch app:" << appName;
}

QString Daemon::getExecutablePath(const QString &appName) {
    // Common paths to search for executables
    QStringList searchPaths = {
        QProcessEnvironment::systemEnvironment().value("PROGRAMFILES"),
        QProcessEnvironment::systemEnvironment().value("PROGRAMFILES(X86)"),
        QProcessEnvironment::systemEnvironment().value("LOCALAPPDATA") + "\\Microsoft\\WindowsApps"
    };
    
    // Common executable names
    QStringList possibleNames = {
        appName.toLower() + ".exe",
        appName.toLower(),
    };
    
    // Search in common locations
    for (const QString &path : searchPaths) {
        QDirIterator it(path, possibleNames, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            if (QFileInfo(filePath).isExecutable()) {
                return filePath;
            }
        }
    }
    
    return QString();
}

QString Daemon::getAppUserModelId(const QString &appName) {
    // Common modern app IDs (this is a simplified version, you might want to expand this)
    QMap<QString, QString> knownApps = {
        {"calculator", "Microsoft.WindowsCalculator_8wekyb3d8bbwe!App"},
        {"notepad", "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"},
        {"paint", "Microsoft.Paint_8wekyb3d8bbwe!App"},
        {"spotify", "SpotifyAB.SpotifyMusic_zpdnekdrzrea0!Spotify"},
        {"chrome", "ChromeHTML"},
        {"firefox", "Firefox-308046B0AF4A39CB"},
        {"code", "Microsoft.VisualStudioCode"},
        {"terminal", "Microsoft.WindowsTerminal_8wekyb3d8bbwe!App"}
    };
    
    return knownApps.value(appName.toLower());
}

void Daemon::send_outputs(const QList<OutputEvent> &vk) {
    qDebug() << "Sending" << vk.count() << "keys";

    QVector<INPUT> inputs;
    inputs.resize(vk.count() * 2); // Press + Release for each key

    for (int i = 0; i < vk.count(); ++i) {
        const OutputEvent &event = vk[i];
        if (const InputEvent *v = std::get_if<InputEvent>(&event)) {
            if (v->type == KeyEventType::AppLaunch) {
                // Handle app launch event
                // Currently not implemented as this is for output events
                continue;
            }
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = int_to_keycode.find_backward(v->keycode);
            // identify key so we can ignore it.
            input.ki.dwExtraInfo = InfoIdentifier;
            if (v->type == KeyEventType::Press) {
                input.ki.dwFlags = 0;
            } else {
                input.ki.dwFlags = KEYEVENTF_KEYUP;
            }
            inputs.append(input);
        } else if (const AppLaunch *app = std::get_if<AppLaunch>(&event)) {
            launchApp(app->appName);
        } else if (const RunScript *script = std::get_if<RunScript>(&event)) {
            qWarning() << "RunScript is not implemented on Windows yet";
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

    qDebug() << "Key pressing:" << kbdStruct->vkCode;

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
