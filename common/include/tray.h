#pragma once

#include <QAction>
#include <QObject>
#include <QString>
#include <QSystemTrayIcon>

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
