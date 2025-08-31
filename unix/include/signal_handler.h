#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H


#include "abstract_signal_handler.h"

/**
 * @brief Unix specific signal handler class to ensure graceful exits of the keybinder program.
 */
class SignalHandler: public AbstractSignalHandler
{
  public:
    /**
     * @brief config_handler Performs all the necessary setup to ensure that the signal
     *        can be properly handled once intercepted and the Qt event loop can gracefully
     *        shut down.
     */
    void config_handler() override;

    /**
     * @brief set_daemon_thread will set the pointer to a daemon_thread
     * @param thread is the QThread object that will be referenced
     */
    void set_daemon_thread(QThread* thread) override;

  private:

    /**
     * @brief daemon_thread is a pointer to the QThread object the daemon will run on.
     *        It will be used to connect logic to the thread.
     */
    QThread* daemon_thread;

    /**
     * @brief pipe_fds The pipe that will be written to during a signal interception
     */
    static int pipe_fds[2];

    /**
     * @brief handle_sig intercepts SIGINT or SIGTERM.
     *        It will write a single byte to a pipe, which is a standard and async-safe
     *        way to signal that something needs to happen. In our case, that 'something'
     *        is gracefully exiting the Qt event loop, which will be done in a connect
     *        function.
     * @param The int representing the signal
     */
    static void handle_sig(int);
};

#endif // SIGNAL_HANDLER_H
