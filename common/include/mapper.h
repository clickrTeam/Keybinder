#ifndef MAPPER_H
#define MAPPER_H
#include "profile.h"
#include "daemon.h"
#include <QDebug>

void setProfile(Profile loaded);
void setDaemon(Daemon* deamon);

// if False let key go through as it's not mapped.
bool mapKeyDownToBind(int virtualKey);
bool mapKeyUpToBind(int virtualKey);

void captureAndRelease();

#endif // MAPPER_H
