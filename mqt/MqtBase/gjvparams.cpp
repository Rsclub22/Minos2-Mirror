#include "gjvparams.h"

GJVParams::GJVParams(QSharedPointer<QFile> f )
      : diskBlock( 1 ), fd( f ), count( 0 )
{}
GJVParams::~GJVParams()
{}
