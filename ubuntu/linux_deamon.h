#ifdef __linux__
#ifndef LINUX_DEAMON_H
#define LINUX_DEAMON_H
#include "../profileInterpreter/profile.h"
#include "../keymap.h"

class LinuxDeamon
{
    public:
        ///
        /// \brief Opens the uinput device to send key events.
        /// \return fd associated with uinput device.
        ///
        int setup_uinput_device();

        ///
        /// \brief Sends key press and release events for a specified key
        /// \param fd: The uinput fd which will send the injected key
        /// \param keycode: The keycode associated with the the event
        ///
        void send_key_event(int fd, int keycode);

        ///
        /// \brief Starts the deamon which allows for key presses to be intercepted
        /// \param activeProfile: The profile which will be loaded into the deamon
        ///
        void linux_start_deamon(Profile activeProfile);

        ///
        /// \brief Closes all fds and cleans up the deamon in order for the keyboard to return to normal function.
        ///
        void cleanup();

    private:
        Profile profile;
        KeyMap map;
};
#endif // LINUX_DEAMON_H
#endif //__linux__
