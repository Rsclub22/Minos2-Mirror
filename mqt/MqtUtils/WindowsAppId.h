#ifndef WINDOWSAPPID_H
#define WINDOWSAPPID_H

#include <QtGlobal>
#include <QWidget>
#include <windows.h>

void setWinAppId(QWidget *w, QString id);
void clearWinAppId(QWidget *w);
#endif // WINDOWSAPPID_H
