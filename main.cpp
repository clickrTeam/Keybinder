#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>

int main(int argc, char *argv[])
{
    QStringList arguments = QCoreApplication::arguments();
    // TODO: We need to add this argument to startup locations. i.e. add to windows registery with exe location.
    bool isStartup = arguments.contains("--startup");

    if (isStartup) {
        QCoreApplication a(argc, argv); // Core application runs minimal - may not be needed
        qDebug() << "App started from system startup.";
        // Run deamon, no visuals

        return a.exec();
    } else {
        QApplication a(argc, argv);
        qDebug() << "App started manually.";
        // Run UI
        MainWindow w;
        w.show();

        return a.exec();
    }
}
