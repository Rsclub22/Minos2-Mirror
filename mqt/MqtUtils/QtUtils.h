#ifndef QTUTILS_H
#define QTUTILS_H
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define  QASCONST qAsConst
#else
#define QASCONST std::as_const
#endif

#endif // QTUTILS_H
