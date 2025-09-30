#include "tray.h"
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>

Tray::Tray(QObject *parent) : QObject(parent) {
    trayIcon = new QSystemTrayIcon(this);

    QIcon icon = QIcon::fromTheme("dialog-information");
    if (icon.isNull()) {
        icon = QIcon(":/icons/myicon.png"); // fallback resource icon
    }
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("Clickr");

    // Create menu
    QMenu *menu = new QMenu();

    // Pause/Resume action
    pauseResumeAction = new QAction("Pause", this);
    connect(pauseResumeAction, &QAction::triggered, this, [this]() {
        if (pauseResumeAction->text() == "Pause") {
            pauseResumeAction->setText("Resume");
            emit paused();
        } else {
            pauseResumeAction->setText("Pause");
            emit resumed();
        }
    });

    // Exit action
    QAction *exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, this, [this]() {
        emit shutdown();
    });

    // Add actions to menu
    menu->addAction(pauseResumeAction);
    menu->addAction(exitAction);

    trayIcon->setContextMenu(menu);

    trayIcon->show();
}

void Tray::sendNotification(const QString &title, const QString &message) {
    if (trayIcon->isVisible()) {
        trayIcon->showMessage(title, message, QSystemTrayIcon::Information,
                              5000);
    }
}
