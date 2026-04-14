#ifndef ICALLSIGNNETWORK_H
#define ICALLSIGNNETWORK_H

#include <QString>

class ICallsignNetwork
{
public:
    virtual ~ICallsignNetwork() = default;

    virtual void requestCallsign(const QString& call) = 0;
};

#endif // ICALLSIGNNETWORK_H
