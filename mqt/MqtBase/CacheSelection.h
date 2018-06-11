#ifndef CACHESELECTION_H
#define CACHESELECTION_H

#include "mwin.h"

class CacheSelection
{
    QMap<QString, MinosStringItem<QString> > _selected; // map of contests by logger (uuids)
    bool dirty;
public:
    CacheSelection();
    ~CacheSelection();
    bool isDirty() const;
    void setDirty();
    void clearDirty();
    QJsonValue pack() const;
    void unpack(const QJsonValue &json);
    MinosStringItem<QString> selected(const QString loggerUuid) const;
    void setSelection(const QString loggerUuid, QString s);
    QStringList getSelectedLoggers();
    QString getSelectedContest(QString loggerid);
};

#endif // CACHESELECTION_H
