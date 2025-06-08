#ifndef ANTENNASTATE_H
#define ANTENNASTATE_H

#include "CacheSelection.h"

class AntennaState
{
    MinosStringItem<QString> _bearing;
    MinosStringItem<QString> _status;
    MinosStringItem<QString> _skyScanNextStep;
    MinosStringItem<QString> _skyScanCountDown;
    MinosItem<int> _skyScanButtonState;
    MinosItem<bool> _skyScanReverseScan;
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

    MinosStringItem<QString> skyScanNextStep() const;
    MinosStringItem<QString> skyScanCountDown() const;
    MinosItem<int> skyScanButtonState() const;
    MinosItem<bool> skyScanReverseScan() const;

    void setSkyScanNextStep(const QString &skyScanNextStep);
    void setSkyScanCountDown(const QString &skyScanCountDown);
    void setSkyScanButtonState(int state);
    void setSkyScanReverseScan(bool reverseScan);

};

#endif // ANTENNASTATE_H
