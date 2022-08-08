#ifndef BASELOGLIST_H
#define BASELOGLIST_H

#include <QObject>

class BaseLogList: public QObject
{
public:
    BaseLogList();
    BaseLogList(const BaseLogList &);
    virtual ~BaseLogList();
};

#endif // BASELOGLIST_H
