#ifndef DEAMON_H
#define DEAMON_H
#include <windows.h>
#include <string>
#include "../profileInterpreter/profile.h"

void winStartDeamon(Profile activeProfile);
void winDeamon();
void cleanup();
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam);
std::string GetKeyName(unsigned int virtualKey);

#endif // DEAMON_H
