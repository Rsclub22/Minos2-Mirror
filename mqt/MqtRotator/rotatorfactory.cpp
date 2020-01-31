#include "rotatorfactory.h"
#include "hamlibRotcontrol.h"

#if defined (WIN32)
#include "pstRotControl.h"
#endif

RotatorFactory::RotatorFactory(QObject *parent) : QObject(parent)
{
    HamlibRotControl::register_rotators(&rotatorsList);

#if defined (WIN32)
    // PSTRotator is only on Windows
    PstRotControl::register_rotators(&rotatorsList, PSTRotatorId);
#endif

}

RotatorFactory::~RotatorFactory()
{

}
