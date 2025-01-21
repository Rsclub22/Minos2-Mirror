#include "rotcapabilities.h"


RotCapabilities::RotCapabilities()
{
    clear();
}



void RotCapabilities::clear()
{
    modelNumber = 0;
    portType = RotCapConstants::none;
    rotatorManufacturer = "";
    rotatorModelName = "";
    supportCwCCwCmd = false;
    supportStopCommand = true;
    minRot = COMPASS_MIN0;
    maxRot = COMPASS_MAX360;
    enableSelectDisplayDial= RotCapConstants::disableSelectDisplayDial;
    pollData = RotCapConstants::pollDataOn;
    allowSouthStopConfig = true;
    allowSkyScan = false;
}




void RotCapabilities::setModelNumber(int modelNumber_)
{
    modelNumber = modelNumber_;
}
int RotCapabilities::getModelNumber() const
{
    return modelNumber;
}

void RotCapabilities::setPortType(RotCapConstants::PortType portType_)
{
    portType = portType_;
}
RotCapConstants::PortType RotCapabilities::getPortType() const
{
    return portType;
}

void RotCapabilities::setRotatorManufacturer(QString rotatorManufacturer_)
{
    rotatorManufacturer = rotatorManufacturer_;
}
QString RotCapabilities::getRotatorManufacturer() const
{
    return rotatorManufacturer;
}


void RotCapabilities::setRotatorModelName(QString rotatorModelName_)
{
    rotatorModelName = rotatorModelName_;
}
QString RotCapabilities::getRotatorModelName() const
{
    return rotatorModelName;
}

void RotCapabilities::setSupportCwCCwCmd(bool supportCwCCwCmd_)
{
    supportCwCCwCmd = supportCwCCwCmd_;
}
bool RotCapabilities::getSupportCwCCwCmd() const
{
    return supportCwCCwCmd;
}

void RotCapabilities::setSupportStopCommand(bool supportStopCommand_)
{
    supportStopCommand = supportStopCommand_;
}
bool RotCapabilities::getSupportStopCommand() const
{
    return supportStopCommand;
}


void RotCapabilities::setMinRot(int minRot_)
{
    minRot = minRot_;
}
int RotCapabilities::getMinRot() const
{
    return minRot;
}

void RotCapabilities::setMaxRot(int maxRot_)
{
    maxRot = maxRot_;
}
int RotCapabilities::getMaxRot() const
{
    return maxRot;
}

void RotCapabilities::setEnableSelectDisplayDial(bool enableSelectDisplayDial_)
{
    enableSelectDisplayDial = enableSelectDisplayDial_;
}
bool RotCapabilities::getEnableSelectDisplayDial() const
{
    return enableSelectDisplayDial;
}

void RotCapabilities::setPollData(bool pollData_)
{
    pollData = pollData_;
}
bool RotCapabilities::getPollData() const
{
    return pollData;
}

void RotCapabilities::setAllowSouthStopConfig(bool allowSouthStopConfig_)
{
    allowSouthStopConfig = allowSouthStopConfig_;
}
bool RotCapabilities::getAllowSouthStopConfig() const
{
    return allowSouthStopConfig;
}
void RotCapabilities::setAllowSkyScan(bool allowSkyScan_)
{
    allowSkyScan = allowSkyScan_;
}
bool RotCapabilities::getAllowSkyScan() const
{
    return allowSkyScan;
}
