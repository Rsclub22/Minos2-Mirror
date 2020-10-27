#ifndef BASELOGLIST_H
#define BASELOGLIST_H

#include "XMPP_pch.h"

class BaseLogList: public QObject
{
public:
    BaseLogList();
    BaseLogList(const BaseLogList &);
    virtual ~BaseLogList();
};

#endif // BASELOGLIST_H
