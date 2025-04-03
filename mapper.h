#ifndef MAPPER_H
#define MAPPER_H
#include "profileInterpreter/profile.h"
#include <QDebug>

void setProfile(Profile loaded);

// if False let key go through as it's not mapped.
bool mapKeyDownToBind(int virtualKey);
bool mapKeyUpToBind(int virtualKey);

#endif // MAPPER_H
