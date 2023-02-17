#ifndef RIGSETUPFORM_H
#define RIGSETUPFORM_H

#include <QWidget>
#include "transvertsetupform.h"
#include "rigcommon.h"
#include "ui_rigsetupform.h"
#include "rigfactory.h"


namespace Ui {
    class rigSetupForm;
}


class SupCheckBoxData
{
public:

    SupCheckBoxData()
    {
        supBandChkBox = nullptr;
    }

    QCheckBox *supBandChkBox;
    QString bandType;
};



class RigSetupForm : public QWidget
{
    Q_OBJECT

public:
    explicit RigSetupForm(RigFactory* rigFactory_, QSharedPointer<scatParams> _radioData,
                          const QVector<QSharedPointer<BandInfo> > _bands, QLogTabWidget* _radioTab, QWidget *parent = nullptr);
    ~RigSetupForm();

    QSharedPointer<scatParams> getRadioData() const;

    QVector<QSharedPointer<BandInfo> > bands;

    QMap<QString, TransVertSetupForm*> transVertTab;


    void addTransVertTab(int tabNum, QString tabName, bool tabChanged);

    QString getRadioModel() const;
    void setRadioModel(QString m);

    void civSetToolTip();
    QString getCIVAddress() const;
    void setCIVAddress(QString v);
    void enableCIVEdit(bool enable);

    QString getComport() const;
    void setComport(QString p);

    QString getDataSpeed() const;
    void setDataSpeed(QString d);

    int comportAvial(QString comport);

    QString getDataBits() const;
    void setDataBits(QString d);

    QString getStopBits() const;
    void setStopBits(QString stop);

    void setParityBits(int b);

    QString getHandshake() const;
    void setHandshake(int h);

    QString getNetAddress() const;
    void setNetAddress(QString netAdd);

    QString getNetPortNum() const;
    void setNetPortNum(QString p);

    QString getRttyMode() const;
    void setRttyMode(QString p);

    QString getPskMode() const;
    void setPskMode(QString p);

    QString getMgmMode() const;
    void setMgmMode(QString p);

    QString getPollInterval() const;
    void setPollInterval(QString i);
    void pollIntervalVisible(bool s);

    void networkDataEntryVisible(bool v);
    void serialDataEntryVisible(bool v);
    void advancedSerialDataEntryVisible(bool v);


    bool getTransVertSelected() const;
    void setTransVertSelected(bool flag);

    void CIVEditVisible(bool visible);
    void transVertTabEnable(bool visible);


    void transVertTabRemove(int tabNum);


    void setTransVertTabText(int tabNum, QString tabName);

    void setAppName(QString name);


    void setEnableRigDataEntry(bool enable);

    void setupRadioModel(QString radioModel);

    void loadTransVertTab(QString transvertName);
    bool getEnableTransVertSw() const;
    void setEnableTransVertSw( bool b);
    void setEnableTransVertSwBoxVisible( bool visible);
    bool getEnableLocalTransVertSw() const;
    void setEnableLocalTransVertSw(bool b);
    void setEnableLocalTransVertSwVisible(bool visible);
    QString getLocTVSwComport() const;
    void setLocTVSWComportVisible(bool visible);
    void setLocTVSwComport(QString p);

    void setTransVertSwVisible(bool b);

    void loadRadioComports();



    void setCurrentRadioName(QString name);
    QString getRigctldNetworkAddress() const;
    void setRigctldNetworkAddress(const QString &address);
    QString getRigctldPortNumber() const;
    void setRigctldPortNumber(const QString &port);
    void setUseRigctldCheckbox(bool checked);
    void rigCtldItemsVisible(bool enable);


    void setRigctldCheckBoxVisible(bool visible);
    void setForceDTRComboBox(int n);
    void setForceRTSComboBox(int n);

    void setForceRTSDisabled(bool state);
    void setForceDTRDisabled(bool state);


    void setSupportBandChkBox(int i, bool checked);
    void setSupportBandChkBox(QString band, bool checked);
    void setSupportBandCheckBoxVisible(bool visible);

    void checkAdvancedCommsCheckBox(bool checked);
    void setAdvancedCommsFlag(bool state);
    void setAdvancedCommsChkBoxVisible(bool visible);
    void setStartMinosRigctldCheckbox(bool checked);

    bool isAnySupportBandChecked();

    void setPttControlsVisible(bool visible);

    void setPTTCheckBoxChecked(bool checked);
    void loadAvailPttComports();
    void setPttComport(QString p);
    void setPttTypeRadioButtons(int type);
    void setPTTCheckBoxDisabled(bool disabled);
    void loadEnableShowCatFeaturesBox(const RigCapabilities rigCap);


    bool isPttComportEqualCatComport() const;
    void pttComportSelDisabled(bool state);


    void setPttComportToolTip(QString toolTip);

    void setPttInitialState();
    void setEnableDisableCatFeaturesGroupVisible(bool visible);
public slots:
    void comSpeedSelected();
    void comDataBitsSelected();
    void comStopBitsSelected();
    void comParitySelected(int);
    void on_forceRTSSelected();

signals:
    void transVertTabAdded(int);


private slots:


    void radioModelSelected();
    void comportSelected();

    void comHandShakeSelected();
    void networkAddressSelected();
    void networkPortSelected();
    void pollIntervalSelected();
    void mgmModeSelected();
    void rttyModeSelected();
    void pskModeSelected();


    void addTransVerter();
    void removeTransVerter();
    void changeBand();
    void enableTransVertSelected(bool);
    void civAddressFinished();



    void enableTransVertSwSel(bool);
    void localTransVertSwSel(bool);
    void locTVComPortSel(int);

    void useRigCtldSelected(bool selected);
    void rigCtldNetworkAddressSelected();
    void rigCtldNetworkPortSelected();
    void on_forceDTRSelected();


    void onAdvancedCommsSelected(bool selected);
    void onStartMinosRigCtldChkBox(bool);

    void onSupbandCheckBoxStateChanged(int i, int state);



    void onPttEnableSelected(bool checked);
    void onPttCatEnableClicked(bool checked);
    void onPttDtrEnableClicked(bool checked);
    void onPttRtsEnableClicked(bool checked);
    void onPttComportSelActivated(int idx);



    void onEnableRitClicked();
    void onEnableSMeterClicked();
    void onEnableVolClicked();
    void onEnableCatPttClicked();
    void onEnableVoiceTxMemClicked();
    void onEnableCwTxMemClicked();
    void onEnableCatFeaturesClicked();
private:




    Ui::rigSetupForm *ui;
    RigFactory *rigFactory;


    QSharedPointer<scatParams> radioData;

    QLogTabWidget* ui_RadioTab;

    QString appName;

    QString currentRadioName;

    QList<QCheckBox*> allSupBandsChkBoxList;

    QMap<QString, SupCheckBoxData> allSupBandsChkBoxesMap;


    void fillHandShakeInfo();
    void fillParityInfo();
    void fillStopBitsInfo();
    void fillDataBitsInfo();
    void fillSpeedInfo();
    //void fillPortsInfo();
    void fillForceLinesInfo();
    void fillRadioModelInfo();
    void fillMgmModes();


    void fillPollInterValInfo();

    bool checkTransVerterNameMatch(QString transVertName);



    bool radioSupportRit(int radioModelNumber);
    void processPortNumber(QLineEdit *netAddBox, QLineEdit *netPortBox, QString &portNumber);


    void rigCtldNetworkAddBoxVisible(bool visible);
    void rigCtldPortBoxVisible(bool visible);



    void initSupBandsChkBoxs();
    void setSupportBandFlag(int i, bool checked);





    void setPttRTSDisabled(bool state);
    void setPttDTRDisabled(bool state);
};

#endif // RIGSETUPFORM_H
