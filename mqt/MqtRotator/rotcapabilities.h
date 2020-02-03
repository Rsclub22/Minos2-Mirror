#ifndef ROTCAPABILITIES_H
#define ROTCAPABILITIES_H

#include "rotatorcommon.h"

namespace RotCapContstants
{
    enum PortType {none, serial, network, usb};
    enum RotatorDisplay {displayFull = true, displayPart = false};
    enum PollData {pollDataOn = true, pollDataOff = false};

}

class RotCapabilities
{
public:
    RotCapabilities(int modelNumber = 0,
                    RotCapContstants::PortType portType_ = RotCapContstants::none,
                    QString rotatorManufacturer_ = "",
                    QString rotatorModelName_ = "",
                    bool supportCwCCwCmd_ = false,
                    int minRot_ = COMPASS_MIN0,
                    int maxRot_ = COMPASS_MAX360,
                    bool rotDisplay_= RotCapContstants::displayFull,
                    bool pollData_ = RotCapContstants::pollDataOn);


    int modelNumber;
    RotCapContstants::PortType portType;
    QString rotatorManufacturer;
    QString rotatorModelName;
    bool supportCwCCwCmd;
    int minRot;
    int maxRot;
    bool rotDisplay;
    bool pollData;


};

#endif // ROTCAPABILITIES_H
