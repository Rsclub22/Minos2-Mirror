#ifndef ANTENNASTATE_H
#define ANTENNASTATE_H

#include "CacheSelection.h"

class AntennaState
{
    MinosStringItem<QString> _bearing;
    MinosStringItem<QString> _status;
    CacheSelection _selected;
public:
    AntennaState();
    AntennaState(QString);
    ~AntennaState()
    {}

    QString pack() const ;
    void unpack(QString) ;
    bool isDirty() const;
    void clearDirty();
    void setDirty();

    MinosStringItem<QString> bearing() const;
    MinosStringItem<QString> status() const;
    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    void setBearing(const QString &bearing);
    void setStatus(const QString &status);
    void setSelected(const QString &loggeruuid, const QString &selected);
    QStringList getSelectedLoggers();
};

#endif // ANTENNASTATE_H
