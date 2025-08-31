#include "signal_handler.h"
#include <windows.h>
#include <QCoreApplication>
#include <QMetaObject>
#include <QDebug>

/// @todo We may want to handle closing the daemon thread explicitly on Windows but I'm not sure. We have to on Linux
BOOL WINAPI SignalHandler::console_ctrl_handler(DWORD ctrlType)
{
    return TRUE;
}

void SignalHandler::set_daemon_thread(QThread* thread)
{
    this->daemon_thread = thread;
}

void SignalHandler::config_handler()
{
}
