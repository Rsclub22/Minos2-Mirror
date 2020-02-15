#ifndef ROTCAPABILITIES_H
#define ROTCAPABILITIES_H

#include "rotatorcommon.h"

namespace RotCapConstants
{
    enum PortType {none, serial, network, usb};
    enum RotatorDisplay {displayFull = true, displayPart = false};
    enum PollData {pollDataOn = true, pollDataOff = false};

}

class RotCapabilities
{
public:
    RotCapabilities(int modelNumber = 0,
                    RotCapConstants::PortType portType_ = RotCapConstants::none,
                    QString rotatorManufacturer_ = "",
                    QString rotatorModelName_ = "",
                    bool supportCwCCwCmd_ = false,
                    int minRot_ = COMPASS_MIN0,
                    int maxRot_ = COMPASS_MAX360,
                    bool rotDisplay_= RotCapConstants::displayFull,
                    bool pollData_ = RotCapConstants::pollDataOn);


    int modelNumber;
    RotCapConstants::PortType portType;
    QString rotatorManufacturer;
    QString rotatorModelName;
    bool supportCwCCwCmd;
    int minRot;
    int maxRot;
    bool rotDisplay;
    bool pollData;


};

#endif // ROTCAPABILITIES_H
