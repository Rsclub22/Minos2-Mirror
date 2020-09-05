#ifndef DELAYEDACTION_H
#define DELAYEDACTION_H

#include "mqtUtils_pch.h"

extern void delayedAction(QObject *p, std::function<void()> pred, int t = 100);

#endif // DELAYEDACTION_H
