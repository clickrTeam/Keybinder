#pragma once
#include "abstract_daemon.h"
#include "key_channel.h"
#include <QDebug>
#include <qglobalstatic.h>
#include <windows.h>

class Daemon : public AbstractDaemon {
  public:
    // Constructor and Destructor
    Daemon(KeySender);
    ~Daemon();

    // Override abstract class methods
    void start() override;
    void cleanup() override;
    void send_outputs(const QList<OutputEvent> &vk) override;
    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ShellProc(int nCode, WPARAM wParam, LPARAM lParam);
    void launchApp(const QString &appName);

private:
    static QString getExecutablePath(const QString &appName);
    static QString getAppUserModelId(const QString &appName);
};
