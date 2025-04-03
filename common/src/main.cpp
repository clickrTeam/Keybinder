// #include "startup.h"
#include "daemon.h"
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>

int main(int argc, char *argv[]) {
    // TODO: We need to add this argument to startup locations. i.e. add to
    // windows registery with exe location.
    QCoreApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();

#ifdef _WIN32

#include "daemon.h"
#elifdef __linux__
#elifdef __APPLE__
#else
#error "Platform not supported"
#endif
    bool isOsStartup = arguments.contains("--osstartup");

    if (isOsStartup) {
        qDebug() << "App started from system startup.";
    } else {
        qDebug() << "App started manually.";
    }
    startUp(isOsStartup);
    return a.exec();
}
