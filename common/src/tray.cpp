#include "tray.h"
#include <QIcon>

Tray::Tray(QObject *parent) : QObject(parent) {
    trayIcon = new QSystemTrayIcon(this);

    QIcon icon = QIcon::fromTheme("dialog-information");
    if (icon.isNull()) {
        icon = QIcon(":/icons/myicon.png"); // fallback resource icon
    }
    trayIcon->setIcon(icon);

    trayIcon->show();
}

void Tray::sendNotification(const QString &title, const QString &message) {
    if (trayIcon->isVisible()) {
        trayIcon->showMessage(title, message, QSystemTrayIcon::Information,
                              5000);
    }
}
