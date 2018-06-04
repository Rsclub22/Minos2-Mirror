#include "CacheSelection.h"

CacheSelection::CacheSelection()
{}

CacheSelection::~CacheSelection()
{}

bool CacheSelection::isDirty() const
{
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
    QMapIterator<QString, MinosStringItem<QString> > iter(_selected);
    while(iter.hasNext())
    {
        iter.next();
        _selected[iter.key()].setDirty();
    }
}

void CacheSelection::clearDirty()
{
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
    QJsonArray namearray = json.toArray();
    for (int i = 0; i < namearray.count(); i++)
    {
        QJsonObject selstruct = namearray[i].toObject();
        QString logger = selstruct.value("logger").toString();
        QString contest = selstruct.value("contest").toString();
        MinosStringItem<QString> s;
        s.setValue(contest);
        _selected[logger] = s;
    }

}

MinosStringItem<QString> CacheSelection::selected(const QString loggerUuid) const
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
        return _selected[loggerUuid];
    }
    MinosStringItem<QString> s;
    return s;
}

void CacheSelection::setSelection(const QString loggerUuid, QString s)
{
    MinosStringItem<QString> m;
    m.setValue(s);
    _selected[loggerUuid] = m;
}
