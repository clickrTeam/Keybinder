// This each OS should have this daemon.h file in their include path so that
// the compiler will just use the right implementation at runtime.
// See mac/include/daemon.h for an example.
#include "daemon.h"
#include "generic_indicator.h"
#include "key_channel.h"
#include "key_counter.h"
#include "local_server.h"
#include "logger.h"
#include "mapper.h"
#include "settings.h"
#include "signal_handler.h"
#include "tray.h"
#include <QApplication>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QStringList>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <csignal>
#include <qcoreapplication.h>
#include <qlogging.h>
#include <qtmetamacros.h>
#include <QLockFile>

QThread *daemon_thread;
QThread *mapper_thread;

void cleanShutdown() {
    qDebug() << "Shutting down...";

    // Ask threads to quit gracefully
    if (daemon_thread) {
        daemon_thread->requestInterruption();
        daemon_thread->quit();
        daemon_thread->wait();
    }

    if (mapper_thread) {
        mapper_thread->requestInterruption();
        mapper_thread->quit();
        mapper_thread->wait();
    }

    QApplication::quit();
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QString lockFilePath = QDir::temp().absoluteFilePath("clickr.lock");
    QLockFile lockFile(lockFilePath);
    lockFile.setStaleLockTime(0); // Never consider stale locks valid

    if (!lockFile.tryLock()) {
        qCritical() << "Tried to run keybinder but another instance is already running.";
        return 1;
    }

    QString path = "empty";
    KeybinderSettings settings;
    KeyCounter key_counter;
    Profile profile;

#ifndef QT_DEBUG
    qInstallMessageHandler(myMessageHandler);
#endif

    if (argc < 2) {
        qDebug() << "No profile argument provided";
        try {
            auto profile_opt = Profile::load_latest();
            if (!profile_opt) {
                qDebug()
                    << "Could not load latest profile, falling back to default";
                profile = Profile::default_profile();
            } else {
                profile = *profile_opt;
            }
        } catch (const std::exception &ex) {
            qDebug() << "Exception loading latest profile:" << ex.what()
                     << " — falling back to default";
            profile = Profile::default_profile();
        } catch (...) {
            qDebug() << "Unknown exception loading latest profile — falling "
                        "back to default";
            profile = Profile::default_profile();
        }
    } else {
        path = argv[1];
        auto profile_opt = Profile::from_file(path);
        if (!profile_opt) {
            qWarning() << "profile:" << path << "could not be loaded";
            std::exit(1);
        }
        profile = *profile_opt;
    }

    SignalHandler sh;

    auto [sender, receiver] = create_channel();
    Daemon daemon(sender);
    // Mapper mapper(activeProfile, daemon, receiver);
    Mapper mapper(
        profile, daemon, receiver, settings, key_counter, [](auto layer_name) {
            QTimer::singleShot(0, qApp, [layer_name]() {
                new GenericIndicator(QString(layer_name),
                                     GenericIndicator::BOTTOM_RIGHT, 1000);
            });
        });

    // I am not sure we will want to use qthreads in this context. A std::thread
    // may be better as it does not run an event loop which I could imagine
    // causing slowdowns
    daemon_thread = QThread::create([&] { daemon.start(); });
    mapper_thread = QThread::create([&] { mapper.start(); });
    daemon_thread->start(QThread::Priority::TimeCriticalPriority);
    mapper_thread->start(QThread::Priority::TimeCriticalPriority);
    sh.set_daemon_thread(daemon_thread);
    sh.config_handler();

    // Somehow hope this works, many varibles can make it not. Working is not so
    // important.
    Logger logger;
    QObject::connect(QCoreApplication::instance(),
                     &QCoreApplication::aboutToQuit,
                     [&logger]() { logger.cleanUp(); });

    // Shutdown when the signal is sent
    Tray tray;
    QObject::connect(&tray, &Tray::shutdown, [&]() { cleanShutdown(); });

    QObject::connect(&settings, &KeybinderSettings::settings_changed, [&]() {
        if (!settings.get_log_key_frequency()) {
            key_counter.clear();
        }
    });

    // Start the local server by calling its constructor (could add start method
    // IDK if needed)
    LocalServer server(mapper, settings, key_counter);

    // Removing for prototype as not yet used
    //
    // TODO: We need to add this argument to startup locations. i.e. add to
    // windows registery with exe location.
    // QStringList arguments = QCoreApplication::arguments();
    // bool isOsStartup = arguments.contains("--osstartup");
    //
    // if (isOsStartup) {
    //     qDebug() << "App started from system startup.";
    // } else {
    //     qDebug() << "App started manually.";
    // }

    return a.exec();
}
