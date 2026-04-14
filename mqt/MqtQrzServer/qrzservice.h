#ifndef QRZSERVICE_H
#define QRZSERVICE_H



#include "CallsignService.h"

class QrzServerMainWindow;

class QRZService : public CallsignService
{
public:
    explicit QRZService(QrzServerMainWindow* owner);

    QString name() const override;

    bool login() override;
    bool lookupCallsign(const QString& call, QrzCallsignData& result) override;

private:
    QrzServerMainWindow* m_owner;
};

#endif // QRZSERVICE_H
