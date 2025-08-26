#include "signal_handler.h"
#include <windows.h>
#include <QCoreApplication>
#include <QMetaObject>
#include <QDebug>

/// @todo We may want to handle closing the daemon thread explicitly on Windows but I'm not sure. We have to on Linux
static BOOL WINAPI Signal_Handler::console_ctrl_handler(DWORD ctrlType)
{
    auto app = QCoreApplication::instance();
    switch (ctrlType) {
    case CTRL_C_EVENT:
        qInfo() << "Ctrl+C received";
        break;

    case CTRL_BREAK_EVENT:
        qInfo() << "Ctrl+Break received";
        break;

    case CTRL_CLOSE_EVENT:
        qInfo() << "Console window is closing";
        break;

    case CTRL_LOGOFF_EVENT:
        qInfo() << "User is logging off";
        break;

    case CTRL_SHUTDOWN_EVENT:
        qInfo() << "System is shutting down";
        break;

    default:
        // Let default handler run for other events
        return FALSE;
    }

    // quit() on the Qt main loop thread
    QMetaObject::invokeMethod(
        app,
        "quit",
        Qt::DirectConnection
        );
    return TRUE;
}

void Signal_Handler::set_daemon_thread(QThread* thread)
{
    this->daemon_thread = thread;
}

void Signal_Handler::config_handler()
{
    // Install our console control handler
    if (!SetConsoleCtrlHandler(consoleCtrlHandler, TRUE)) {
        qWarning() << "Failed to install console control handler";
    }
}
