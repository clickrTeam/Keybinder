#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H


#include "abstract_signal_handler.h"
#include <windows.h>
/**
 * @brief Windows specific signal handler class to ensure graceful exits of the keybinder program.
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
     * @brief Logs which console control type is recieved and invokes Qt's quit
     *        logic to ensure graceful exitting of the event loop.
     */
    static BOOL WINAPI console_ctrl_handler(DWORD ctrlType);
};

#endif // SIGNAL_HANDLER_H
