#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H


#include "abstract_signal_handler.h"

/**
 * @brief Linux specific signal handler class to ensure graceful exits of the keybinder program.
 */
class Signal_Handler: public Abstract_Signal_Handler
{
  public:
    /**
     * @brief config_handler Performs all the necessary setup to ensure that the signal
     *        can be properly handled once intercepted and the Qt event loop can gracefully
     *        shut down.
     */
    void config_handler() override;

  private:
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
