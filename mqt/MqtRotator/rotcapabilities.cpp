#include "rotcapabilities.h"


RotCapabilities::RotCapabilities(int modelNumber_,
                                 RotCapConstants::PortType portType_,
                                 QString rotatorManufacturer_,
                                 QString rotatorModelName_,
                                 bool supportCwCCwCmd_ ,
                                 int minRot_ ,
                                 int maxRot_ ,
                                 bool enableSelectDisplayDial_,
                                 bool pollData_ )

               : modelNumber (modelNumber_),
                 portType (portType_),
                 rotatorManufacturer (rotatorManufacturer_),
                 rotatorModelName (rotatorModelName_),
                 supportCwCCwCmd (supportCwCCwCmd_),
                 minRot (minRot_),
                 maxRot (maxRot_),
                 enableSelectDisplayDial (enableSelectDisplayDial_),
                 pollData (pollData_)

{

}
