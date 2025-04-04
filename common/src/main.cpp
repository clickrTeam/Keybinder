// This each OS should have this daemon.h file in their include path so that
// the compiler will just use the right implementation at runtime.
// See mac/include/daemon.h for an example
#include "abstract_daemon.h"
#include "readprofile.h"
#include "daemon.h"
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>

int main(int argc, char *argv[]) {
    // TODO: We need to add this argument to startup locations. i.e. add to
    // windows registery with exe location.
    QCoreApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();
    Daemon d;
    d.start();

    Profile activeProfile = proccessProfile("../../exampleProfiles/e2.json");

    bool isOsStartup = arguments.contains("--osstartup");

    if (isOsStartup) {
        qDebug() << "App started from system startup.";
    } else {
        qDebug() << "App started manually.";
    }
    return a.exec();
}
