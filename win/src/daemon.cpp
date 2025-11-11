#include "daemon.h"
#include "event.h"
#include "key_channel.h"
#include "key_map.h"
#include "app_cache.h"
#include "generic_indicator.h"
#include <QCoreApplication>
#include <QApplication>
#include <QTimer>
#include <QtCore/qlogging.h>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <windows.h>
#include <winuser.h>
#include <shobjidl.h>
#include <wrl/client.h>
#include <winstring.h>
#include <roapi.h>
#include <thread>

using namespace Microsoft::WRL;

HHOOK kbd = NULL; // Global hook handle
HWINEVENTHOOK event_hook = NULL; // Global event_hook hook handle
KeySender key_sender(nullptr);
const ULONG_PTR InfoIdentifier =
    0x1234ABCD; // allowed collisions otherwise,
                // ((ULONG_PTR)GetCurrentProcessId() << 32) | 0x1234ABCD

// CoInitialize is required for modern app launches
struct COMInitializer {
    COMInitializer() { CoInitialize(nullptr); }
    ~COMInitializer() { CoUninitialize(); }
} com_initializer;

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

    event_hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
                           NULL, WinEventProc, 0, 0,
                           WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    // idHook, HookProc, Hinstance - N/I, dwThreadId - N/I
    // hooks idHook to HookProc, this happens before the os processes the input
}

void CALLBACK Daemon::WinEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd,
                                   LONG idObject, LONG idChild, DWORD, DWORD) {
    if (event != EVENT_SYSTEM_FOREGROUND) return;
    if (idObject != OBJID_WINDOW || idChild != CHILDID_SELF) return;

    wchar_t className[256] = {0};
    wchar_t windowTitle[512] = {0};

    GetClassNameW(hwnd, className, _countof(className));
    GetWindowTextW(hwnd, windowTitle, _countof(windowTitle));

    QString appName = QString::fromWCharArray(windowTitle);
    if (appName.isEmpty()) appName = QString::fromWCharArray(className);

    qDebug() << "Window activated/focused:" << appName;

    InputEvent e = InputEvent::fromApp(appName);
    key_sender.send_key(e);
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
    if (event_hook) {
        UnhookWinEvent(event_hook);
        event_hook = NULL;
        qDebug() << "event_hook hook uninstalled.";
    }
    qDebug() << "Daemon cleaned up";
}

// call-site unchanged
void Daemon::launchApp(const QString &appName) {
    // capture appName by value so the async task owns it
    std::thread([appName, this]() {
        // Initialize COM for this thread
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        bool comInitialized = SUCCEEDED(hr);

        qDebug() << "Trying to launch app via ShellExecute:" << appName;
        AppCache::Entry entry;
        QString appPath; QString appId;

        if (cache.lookup(appName, entry)) {
            appPath = entry.path;
            appId = entry.appId;
        } else {
            // Create the notification for layer switching
            QTimer::singleShot(0, qApp, []() {
                new GenericIndicator("Finding app...", GenericIndicator::BOTTOM_RIGHT, 1000);
            });
            appPath = getExecutablePath(appName);
            appId = getAppUserModelId(appName);
            cache.store(appName, {appPath, appId});
        }
        qDebug() << "App identification received { appPath: " << appPath << ", appId: " << appId << " };";

        if (!appPath.isEmpty()) {
            std::wstring wPath = appPath.toStdWString();
            HINSTANCE result = ShellExecuteW(nullptr, L"open", wPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            if (reinterpret_cast<INT_PTR>(result) > 32) {
                qDebug() << "Successfully launched app via ShellExecute:" << appName;
                if (comInitialized) CoUninitialize();
                return;
            } else {
                qWarning() << "ShellExecute failed with code" << reinterpret_cast<INT_PTR>(result);
            }
        }

        if (!appId.isEmpty()) {
            // Use WindowsCreateString/WindowsDeleteString around HSTRING
            HSTRING appIdHString = nullptr;
            WindowsCreateString(appId.toStdWString().c_str(), static_cast<UINT32>(appId.length()), &appIdHString);

            Microsoft::WRL::ComPtr<IApplicationActivationManager> activationManager;
            hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER,
                                  IID_PPV_ARGS(&activationManager));
            if (SUCCEEDED(hr) && activationManager) {
                DWORD newProcessId = 0;
                UINT32 length = 0;
                PCWSTR appIdStr = WindowsGetStringRawBuffer(appIdHString, &length);
                hr = activationManager->ActivateApplication(appIdStr, nullptr, AO_NONE, &newProcessId);
                if (SUCCEEDED(hr)) {
                    qDebug() << "Successfully launched modern app:" << appName;
                    WindowsDeleteString(appIdHString);
                    if (comInitialized) CoUninitialize();
                    return;
                } else {
                    qWarning() << "ActivateApplication failed HRESULT=" << hr;
                }
            } else {
                qWarning() << "CoCreateInstance failed HRESULT=" << hr;
            }
            if (appIdHString) WindowsDeleteString(appIdHString);
        }

        qWarning() << "Failed to launch app:" << appName;
        if (comInitialized) CoUninitialize();
    }).detach();
}

QString Daemon::getExecutablePath(const QString &appName) {
    // absolute path case
    QFileInfo qfi(appName);
    if (qfi.exists() && qfi.isFile()) return appName;

    // Common paths to search for executables
    QStringList searchPaths = {
        QProcessEnvironment::systemEnvironment().value("PROGRAMFILES"),
        QProcessEnvironment::systemEnvironment().value("PROGRAMFILES(X86)"),
        QProcessEnvironment::systemEnvironment().value("LOCALAPPDATA"),
        QProcessEnvironment::systemEnvironment().value("PROGRAMDATA"),
        QProcessEnvironment::systemEnvironment().value("USERPROFILE") + "\\AppData\\Local",
        QProcessEnvironment::systemEnvironment().value("USERPROFILE") + "\\AppData\\Roaming"
    };

    // add WindowsApps path explicitly if present
    QString windowsApps = QProcessEnvironment::systemEnvironment().value("LOCALAPPDATA") + "\\Microsoft\\WindowsApps";
    if (!searchPaths.contains(windowsApps)) searchPaths.append(windowsApps);
    
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
    static const QMap<QString, QString> knownApps = {
        // Built-in Windows apps (AppUserModelId)
        {"calculator", "Microsoft.WindowsCalculator_8wekyb3d8bbwe!App"},
        {"notepad", "Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"},
        {"paint", "Microsoft.Paint_8wekyb3d8bbwe!App"},
        {"terminal", "Microsoft.WindowsTerminal_8wekyb3d8bbwe!App"},
        {"photos", "Microsoft.Windows.Photos_8wekyb3d8bbwe!App"},
        {"mail", "microsoft.windowscommunicationsapps_8wekyb3d8bbwe!microsoft.windowslive.mail"},
        {"calendar", "microsoft.windowscommunicationsapps_8wekyb3d8bbwe!microsoft.windowslive.calendar"},
        {"edge", "Microsoft.MicrosoftEdge_8wekyb3d8bbwe!MicrosoftEdge"},
        {"store", "Microsoft.WindowsStore_8wekyb3d8bbwe!App"},
        {"voice recorder", "Microsoft.SoundRecorder_8wekyb3d8bbwe!App"},
        {"snippingtool", "Microsoft.ScreenSketch_8wekyb3d8bbwe!App"},
        {"settings", "windows.immersivecontrolpanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel"},
        {"cortana", "Microsoft.549981C3F5F10!App"},

        // Microsoft Office (examples — Office Click-to-Run ProgIDs may vary)
        {"word", "Word.Application"},
        {"excel", "Excel.Application"},
        {"powerpoint", "PowerPoint.Application"},
        {"outlook", "Outlook.Application"},

        // Browsers (ProgIDs / common identifiers)
        {"chrome", "ChromeHTML"},
        {"chrome-beta", "ChromeHTML"},
        {"edge-chromium", "MSEdgeHTM"},
        {"firefox", "FirefoxURL"},
        {"firefox-old", "Firefox-308046B0AF4A39CB"},
        {"brave", "BraveHTML"},
        {"opera", "OperaHTML"},

        // IDEs / Editors
        {"code", "Microsoft.VisualStudioCode"},
        {"vscode", "Microsoft.VisualStudioCode"},
        {"visual studio", "VisualStudio.DTE"}, // ProgID for automation; executable path preferred
        {"sublime", "SublimeText"}, // may not be a ProgID on all installs
        {"notepad++", "Notepad++_file"}, // varies

        // Runtimes / Launchers / Gaming
        {"steam", "Steam"}, // Steam has installers; prefer exe path, but "Steam" can be used
        {"epic", "EpicGamesLauncher"}, // often an exe under Program Files
        {"twitch", "Twitch"}, // older Twitch launcher
        {"curseforge", "CurseForge.exe"}, // prefer exe path or AppPaths registry
        {"battle.net", "Blizzard Battle.net"}, // installers vary
        {"origin", "Origin"}, // EA Origin
        {"uplay", "Ubisoft Game Launcher"},
        {"gog", "GOG Galaxy"},
        {"riot client", "RiotClientServices"}, // may require exe
        {"steam-deck", "steam://open"}, // protocol example

        // Media / Communication
        {"spotify", "SpotifyAB.SpotifyMusic_zpdnekdrzrea0!Spotify"},
        {"vlc", "VLC"}, // often VLC.exe
        {"discord", "Discord"}, // installer dependent; also "discord" protocol
        {"skype", "SkypeApp_4.61.0.77_x64__kzf8qxf38zg5c!App"},
        {"teams", "MicrosoftTeams"}, // desktop has exe
        {"zoom", "Zoom"}, // exe name usually Zoom.exe

        // Common utilities
        {"file explorer", "Explorer.EXE"},
        {"explorer", "Explorer.EXE"},
        {"cmd", "cmd.exe"},
        {"powershell", "PowerShell"}, // older ProgID; prefer exe path/pwsh
        {"powershell core", "pwsh.exe"},
        {"task manager", "Taskmgr.exe"},
        {"registry editor", "regedit.exe"},

        // Development tools / runtimes
        {"docker", "Docker Desktop"},
        {"postgres", "pgAdmin"}, // varies
        {"mysql workbench", "MySQLWorkbench"},

        // Add-ons and protocol examples (can be used with ShellExecute)
        {"mailto", "mailto:"},
        {"skype-protocol", "skype:"},
        {"slack", "slack"},
    };

    auto sanitize = [](const QString &s)->QString{
        QString out;
        out.reserve(s.size());
        for (QChar c : s.toLower()) {
            if (c.isLetterOrNumber()) out.append(c);
            // treat common word separators as nothing (so "snipping tool" -> "snippingtool")
            // ignore punctuation, underscores, hyphens, and spaces
        }
        return out;
    };

    QString key = sanitize(appName);
    return knownApps.value(key);
}

void Daemon::send_outputs(const QList<OutputEvent> &vk) {
    qDebug() << "Sending" << vk.count() << "keys";

    QVector<INPUT> inputs;
    inputs.resize(vk.count() * 2); // Press + Release for each key

    for (int i = 0; i < vk.count(); ++i) {
        const OutputEvent &event = vk[i];
        if (const KeyEvent *v = std::get_if<KeyEvent>(&event)) {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = int_to_keycode.find_backward(v->key());
            // identify key so we can ignore it.
            input.ki.dwExtraInfo = InfoIdentifier;
            if (v->type == KeyEventType::Press) {
                input.ki.dwFlags = 0;
            } else {
                input.ki.dwFlags = KEYEVENTF_KEYUP;
            }
            inputs.append(input);
        } else if (const RunScript *script = std::get_if<RunScript>(&event)) {
            qWarning() << "RunScript is not implemented on Windows yet";
        } else if (const AppLaunch *app = std::get_if<AppLaunch>(&event)) {
            launchApp(app->appName);
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
        KeyEvent e;
        e.keycode = int_to_keycode.find_forward(kbdStruct->vkCode);
        e.type = KeyEventType::Press;
        if (key_sender.send_key(e))
            return 1; // Suppress keypress
        break;
    }
    case WM_KEYUP: {
        KeyEvent e;
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
