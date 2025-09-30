#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QString>
#include <QAction>

class Tray : public QObject {
    Q_OBJECT

public:
    Tray(QObject *parent = nullptr);

signals:
    void paused();
    void resumed();
    void shutdown();

private:
    QSystemTrayIcon *trayIcon;
    QAction *pauseResumeAction;
};
