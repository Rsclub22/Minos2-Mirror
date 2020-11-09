#include "CacheSelection.h"

CacheSelection::CacheSelection()
{}

CacheSelection::~CacheSelection()
{}

bool CacheSelection::isDirty() const
{
    if (dirty)
        return true;
    QMapIterator<QString, MinosStringItem<QString> > iter(_selected);
    while(iter.hasNext())
    {
        iter.next();
        if (iter.value().isDirty())
            return true;
    }
    return false;
}

void CacheSelection::setDirty()
{
    dirty = true;
}

void CacheSelection::clearDirty()
{
    dirty = false;
    QMapIterator<QString, MinosStringItem<QString> > iter(_selected);
    while(iter.hasNext())
    {
        iter.next();
        _selected[iter.key()].clearDirty();
    }
}

QJsonValue CacheSelection::pack() const
{
    QJsonArray sel;
    QMapIterator<QString, MinosStringItem<QString> > iter(_selected);
    while(iter.hasNext())
    {
        iter.next();

        QJsonObject log;
        log.insert("logger", iter.key());
        log.insert("contest", iter.value().getValue());
        sel.append(log);
    }
    return sel;
}

void CacheSelection::unpack(const QJsonValue &json)
{
    // When a selection disappears, we need to do the same

    QVector<QString> notSelected; // map of contests by logger (uuids)
    QMapIterator<QString, MinosStringItem<QString> > iter(_selected);
    while (iter.hasNext())
    {
        iter.next();
        notSelected.push_back(iter.key());
    }
    QJsonArray selarray = json.toArray();
    for (int i = 0; i < selarray.count(); i++)
    {
        QJsonObject selstruct = selarray[i].toObject();
        QString logger = selstruct.value("logger").toString();
        QString contest = selstruct.value("contest").toString();
        _selected[logger].setValue(contest);

        int n = notSelected.indexOf(logger);
        if (n >= 0)
            notSelected.remove(n);
    }

    for (int i = 0; i < notSelected.count(); i++)
    {
        _selected.remove(notSelected[i]);
        setDirty();
    }
}

MinosStringItem<QString> CacheSelection::getSelectedContest(const QString loggerUuid) const
{
    if (loggerUuid.isEmpty())
    {
        QMapIterator<QString, MinosStringItem<QString> > iter(_selected);
        if (iter.hasNext())
        {
            iter.next();
            return iter.value();    // first contest that is selected
        }
    }
    else
    {
        if (_selected.contains(loggerUuid))
        {
            return _selected[loggerUuid];
        }
    }
    MinosStringItem<QString> s;
    return s;
}

void CacheSelection::setSelection(const QString loggerUuid, QString s)
{
    if (s.isEmpty())
    {
        _selected.remove(loggerUuid);
        setDirty();
    }
    else
    {
        MinosStringItem<QString> m;
        m.setValue(s);
        _selected[loggerUuid] = m;
    }
}
QStringList CacheSelection::getSelectedLoggers()
{
    QStringList loggers;
    QMapIterator<QString, MinosStringItem<QString> > iter(_selected);
    if (iter.hasNext())
    {
        iter.next();
        loggers.append( iter.key());
    }
    return loggers;
}


