#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QString>

class Tray : public QObject {
    Q_OBJECT

public:
    Tray(QObject *parent = nullptr);

private:
    QSystemTrayIcon *trayIcon;
};
