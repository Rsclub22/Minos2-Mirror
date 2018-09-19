#ifndef ANTENNASTATE_H
#define ANTENNASTATE_H
#include "base_pch.h"

const QString AntennaStateType("AntennaState");

class AntennaState: public PubSubValue
{
    MinosStringItem<QString> _bearing;
    MinosStringItem<QString> _status;
    CacheSelection _selected;
public:
    AntennaState();
    AntennaState(QString);
    virtual ~AntennaState() override
    {}

    virtual QString pack() const override;
    virtual void unpack(QString) override;
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
