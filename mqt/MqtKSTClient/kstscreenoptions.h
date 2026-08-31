#ifndef KSTSCREENOPTIONS_H
#define KSTSCREENOPTIONS_H

#include <QObject>

#include "ScreenConfigElement.h"

class KSTScreenOptions: public QObject
{
    Q_OBJECT
    static QVector <SCTypeOption> kstScreenOptions;
    static QString defaultConfig;
    static QString protectedConfig;
public:
    KSTScreenOptions();
};

#endif // KSTSCREENOPTIONS_H
