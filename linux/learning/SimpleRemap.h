#ifndef SIMPLEREMAP_H
#define SIMPLEREMAP_H

#include <map>

/* Inline to tell the compiler it is ok if the defintions come across twice.
 * I am not sure why they were coming across twice in the first place. I think it has something to do with "transliation units"
 * Mark remaps as extern so that it can be used in the .cpp file. NOTE: May not be necessary.
 */
inline extern std::map<int, int> remaps;

inline int setup_uinput_device();
inline void send_key_event(int fd, int keycode);
inline void linuxStartDeamon();

#endif // SIMPLEREMAP_H
