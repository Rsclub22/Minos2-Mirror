#ifndef ROTCAPABILITIES_H
#define ROTCAPABILITIES_H

#include "rotatorcommon.h"

namespace RotCapConstants
{
    enum PortType {none, serial, network, usb};
    enum SelectDisplayCompass {enableSelectDisplayDial = true, disableSelectDisplayDial = false};
    enum PollData {pollDataOn = true, pollDataOff = false};

}

class RotCapabilities
{
public:
    RotCapabilities();



    void clear();
    void setModelNumber(int modelNumber_);
    int getModelNumber() const;

    void setPortType(RotCapConstants::PortType portType_);
    RotCapConstants::PortType getPortType() const;

    void setRotatorManufacturer(QString rotatorManufacturer_);
    QString getRotatorManufacturer() const;

    void setRotatorModelName(QString rotatorModelName_);
    QString getRotatorModelName() const;

    void setSupportCwCCwCmd(bool supportCwCCwCmd_);
    bool getSupportCwCCwCmd() const;

    void setSupportStopCommand(bool supportStopCommand_);
    bool getSupportStopCommand() const;

    void setMinRot(int minRot_);
    int getMinRot() const;

    void setMaxRot(int maxRot_);
    int getMaxRot() const;

    void setEnableSelectDisplayDial(bool enableSelectDisplayDial_);
    bool getEnableSelectDisplayDial() const;

    void setPollData(bool pollData_);
    bool getPollData() const;

    void setAllowSouthStopConfig(bool allowSouthStopConfig_);
    bool getAllowSouthStopConfig() const;

    void setAllowSkyScan(bool allowSkyScan_);
    bool getAllowSkyScan() const;


private:

    int modelNumber;
    RotCapConstants::PortType portType;
    QString rotatorManufacturer;
    QString rotatorModelName;
    bool supportCwCCwCmd;
    bool supportStopCommand;
    int minRot;
    int maxRot;
    bool enableSelectDisplayDial;
    bool allowSkyScan;
    bool pollData;
    bool allowSouthStopConfig;

};

#endif // ROTCAPABILITIES_H
