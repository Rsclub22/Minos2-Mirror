#ifndef WINDOWSAPPID_H
#define WINDOWSAPPID_H

#include <QWidget>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
void setWinAppId(QWidget *w, QString id);
void clearWinAppId(QWidget *w);
#endif // WINDOWSAPPID_H
