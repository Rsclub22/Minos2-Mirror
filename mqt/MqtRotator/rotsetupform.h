/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef ROTSETUPFORM_H
#define ROTSETUPFORM_H

#include <QWidget>
#include "rotatorfactory.h"
#include "serialCommonData.h"

namespace Ui {
class rotSetupForm;
}

class rotSetupForm : public QWidget
{
    Q_OBJECT

public:
    explicit rotSetupForm(RotatorFactory *rotFactory_, srotParams* _antennaData, QWidget *parent = nullptr);
    ~rotSetupForm();

    srotParams *antennaData;

    QString getAntennaName();
    void setAntennaName(QString n);

    QString getRotatorModel();
    void setRotatorModel(QString m);


    QString getComport();
    void setComport(QString p);

    QString getDataSpeed();
    void setDataSpeed(QString d);

    QString getDataBits();
    void setDataBits(QString d);

    QString getStopBits();
    void setStopBits(QString stop);

    void setParityBits(int b);

    QString getHandshake();
    void setHandshake(int h);

    QString getNetAddress();
    void setNetAddress(QString netAdd);

    QString getNetPortNum();
    void setNetPortNum(QString p);

    QString getPollInterval();
    void setPollInterval(QString i);

    bool getAntennaValueChanged(){return antennaValueChanged;}
    void setAntennaValueChanged(bool state){antennaValueChanged = state;}

    bool getAntennaNameChanged(){return antennaNameChanged;}
    void setAntennaNameChanged(bool state){antennaNameChanged = state;}

    void fillPortsInfo();

    bool getsStopOffBut();
    void setSStopOffButChecked(bool s);
    void setSStopOffButVisible(bool s);


    bool getRotInvertBut();
    void setRotInvertButChecked(bool s);
    void setRotInvertButVisible(bool s);

    bool getCompassBut();
    void setCompassButChecked(bool s);
    void setCompassButVisible(bool s);

    void sStopButtonsVisible(bool visible);
    void setSStopButtons(southStop stopType);

    bool getCheckOverrun();
    void setCheckOverrun(bool s);

    QString getAntennaOffset();
    void setAntennaOffset(QString o);
    void setOverRunFlagVisible(bool s);

    void serialDataEntryVisible(bool v);
    void networkDataEntryVisible(bool v);

    int comportAvial(QString comport);


    void antennaOffSetVisible(bool s);
    void pollIntervalVisible(bool s);



    void setSimCW_CCWcmdVisible(bool visible);
    void setSimCW_CCWcmdChecked(bool checked);




    void setupRotatorModel(QString rotatorModel);

    bool setEndStopType(srotParams *antennaData, int minRot, int maxRot);
    void setOverlapEndStop(srotParams *antennaData, bool overrunState);

    void setAdvancedCommsFlag(bool state);
    void advancedSerialDataEntryVisible(bool v);
    void checkAdvancedCommsCheckBox(bool checked);
    void setAdvancedCommsChkBoxVisible(bool visible);
    void setForceRTS(int n);
    void setForceRTSDisabled(bool state);
    void on_forceRTSSelected();
    void setForceDTR(int n);
    void on_forceDTRSelected();
private slots:

    void rotatorModelSelected();
    void comportSelected();
    void comDataSpeedSelected();
    void comDataBitsSelected();
    void comStopBitsSelected();
    void comParityBitsSelected();
    void comHandshakeSelected();
    void comNetAddressSelected();
    void comNetPortNumSelected();
    void pollIntervalSelected();
    void overlapSelected();
    void antennaOffSetSelected();
    void simCWCCWCmdSelected();
    void sStopOffButSelected();
    void rotInvertButSelected();
    void compassButSelected();



    void onAdvancedCommsSelected(bool selected);
private:
    Ui::rotSetupForm *ui;

    bool antennaValueChanged;
    bool antennaNameChanged;

    RotatorFactory* rotFactory;

    const int minOffset = -90;
    const int maxOffset = 90;

    void loadComports();
    void loadRotatorModels();
    void loadAntennaName();
    void fillRotatorModelInfo();
    void fillPollInterValInfo();

    void fillSpeedInfo();
    void fillDataBitsInfo();
    void fillStopBitsInfo();
    void fillParityBitsInfo();
    void fillHandShakeInfo();





};

#endif // ROTSETUPFORM_H
