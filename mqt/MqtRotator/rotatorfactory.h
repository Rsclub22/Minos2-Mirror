#ifndef ROTATORFACTORY_H
#define ROTATORFACTORY_H

#include <QObject>
#include "rotatorcommon.h"
#include "rotcapabilities.h"



enum        // supported non-hamlib rotator interfaces
{
    NonHamlibBaseId = 9899,
    PSTRotatorId
};



typedef   QMap<QString, RotCapabilities> Rotators;

class RotatorFactory : public QObject
{
    Q_OBJECT
public:




    explicit RotatorFactory(QObject *parent = nullptr);
    ~RotatorFactory();




    Rotators* supported_rotators();

signals:

private:
    Rotators rotatorsList;


};



#endif // ROTATORFACTORY_H
