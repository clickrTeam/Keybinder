#include "signal_handler.h"
#include <unistd.h>
#include <csignal>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QMetaObject>

int Signal_Handler::pipe_fds[2] = { -1, -1 };

void Signal_Handler::handle_sig(int sig)
{
    char c = 1;
    write(pipe_fds[1], &c, 1);
}

void Signal_Handler::config_handler()
{
    // Create a pipe
    if (pipe(pipe_fds)) {
        qFatal("Failed to create pipe");
    }

    struct sigaction sa{};
    sa.sa_handler   = handle_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags     = SA_RESTART;
    sigaction(SIGINT,  &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    QCoreApplication* app = QCoreApplication::instance();
    QSocketNotifier *notifier = new QSocketNotifier(pipe_fds[0], QSocketNotifier::Read, app);
    QObject::connect(notifier, &QSocketNotifier::activated, [notifier](int)
    {
        notifier->setEnabled(false);
        char c;
        ::read(pipe_fds[0], &c, sizeof(c)); // clear the pipe

        // Exit the event loop
        QCoreApplication::instance()->quit();
     });
}
