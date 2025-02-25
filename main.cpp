#include <QApplication>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>
#include "startup.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();
    // TODO: We need to add this argument to startup locations. i.e. add to windows registery with exe location.
    bool isOsStartup = arguments.contains("--osstartup");

    if (isOsStartup) {
        qDebug() << "App started from system startup.";
    } else {
        qDebug() << "App started manually.";
    }
    startUp(isOsStartup);
    return a.exec();
}
