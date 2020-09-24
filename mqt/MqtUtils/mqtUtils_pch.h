#ifndef mqtUtils_pchH
#define mqtUtils_pchH

#include <cstdint>

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QFont>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QSharedMemory>
#include <QTextStream>
#include <QThread>
#include <QSystemSemaphore>
#include <QTcpSocket>
#include <QSettings>
#include <QSharedPointer>

#include <exception>
#include <string>
#include <algorithm>    // and it also gets included by a different route

#include <stdio.h>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
#include <errno.h>

#include "LogEvents.h"
#include "MTrace.h"
#include "MLogFile.h"
#include "AppStartup.h"
#include "StdInReader.h"
#include "frequency.h"

#endif
