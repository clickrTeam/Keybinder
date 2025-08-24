#ifndef ABSTRACT_SIGNAL_HANDLER_H
#define ABSTRACT_SIGNAL_HANDLER_H

/**
 * @brief The Abstract_Signal_Handler class acts as an interface for OS specific signal handling.
 *        The purpose of this is to ensure a standardized way to gracefully exit keybinder.
 */
class Abstract_Signal_Handler
{
  public:
    virtual ~Abstract_Signal_Handler() = default;

    virtual void config_handler() = 0;
};

#endif // ABSTRACT_SIGNAL_HANDLER_H
