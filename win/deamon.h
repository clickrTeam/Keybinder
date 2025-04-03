#ifdef _WIN32
#ifndef DEAMON_H
#define DEAMON_H
#include <qglobalstatic.h>
#include <windows.h>
#include <QDebug>
#include <windows.h>
#include <winuser.h>

void winStartDeamon();
void winDeamon();
void cleanup();

LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam);

void press(int virtualKey);

#endif // DEAMON_H
#endif
