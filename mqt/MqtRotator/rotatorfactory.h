#ifndef ROTATORFACTORY_H
#define ROTATORFACTORY_H

#include <QObject>
#include "rotatorcommon.h"



enum        // supported non-hamlib rotator interfaces
{
    NonHamlibBaseId = 9899,
    PSTRotatorId
};

class RotatorFactory : public QObject
{
    Q_OBJECT
public:
    explicit RotatorFactory(QObject *parent = nullptr);
    ~RotatorFactory();

    struct RotCapabilities
    {
        enum PortType {none, serial, network, usb};

        explicit RotCapabilities(int modelNumber = 0,
                              PortType portType = none,
                              bool supportCwCCwCmd = false,
                              int minRot = COMPASS_MIN0,
                              int maxRot = COMPASS_MAX360,
                              bool asynchronous = false)

            : modelNumber_ {modelNumber},
              portType_ {portType},
              supportCwCCwCmd_ {supportCwCCwCmd},
              minRot_ {minRot},
              maxRot_ {maxRot},
              asynchronous_ {asynchronous}
    {}

    int modelNumber_;
    PortType portType_;
    bool supportCwCCwCmd_;
    int minRot_;
    int maxRot_;
    bool asynchronous_;



    };

    typedef   QMap<QString, RotCapabilities> Rotators;

signals:

private:
    Rotators rotatorsList;

};

#endif // ROTATORFACTORY_H
