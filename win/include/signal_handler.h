#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H


#include "abstract_signal_handler.h"
#include <windows.h>
/**
 * @brief Windows specific signal handler class to ensure graceful exits of the keybinder program.
 */
class Signal_Handler: public AbstractSignalHandler
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
     * @brief Logs which console control type is recieved and invokes Qt's quit
     *        logic to ensure graceful exitting of the event loop.
     */
    static BOOL WINAPI console_ctrl_handler(DWORD ctrlType);
};

#endif // SIGNAL_HANDLER_H
