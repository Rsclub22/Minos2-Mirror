#ifndef BANDSELBUTTONS_H
#define BANDSELBUTTONS_H

#include <QObject>
#include <QToolButton>
#include <QMap>
#include "BandList.h"
#include "rigcommon.h"
#include "freqpresetdialog.h"




namespace bandSelButtonData
{
    const QString BUTTON_ON_STYLE = QString("background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black; max-width: 20px; max-height: 19px;  padding: 3px;\n");
    const QString BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black; max-width: 20px; max-height: 19px;  padding: 3px;\n");
    const QString TYPE_BUTTON_ON_STYLE = QString("background-color: White ; border-style: outset; border-width: 1px; border-color: black; max-width: 20px; max-height: 19px;  padding: 3px;\n");
    const QString TYPE_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black; max-width: 20px; max-height: 19px;  padding: 3px;\n");
    const QString HF_TAB_NAME = "HF";
    const QString VHF_TAB_NAME = "VHF";
    const QString MW_TAB_NAME = "MW";
    const QString HF_BAND_TYPE = "HF";
    const QString VHF_BAND_TYPE = "VHF";
    const QString MW_BAND_TYPE = "MWAVE";


}


class BandSelButtons : public QObject
{
     Q_OBJECT

public:
    explicit BandSelButtons(const QVector<QSharedPointer<BandInfo> > &bands,  QGridLayout *_bandSelGrid, QObject *parent = nullptr);



    void setButtonVisible(QString band, bool visible);
    void setHf(bool state);

    int setButtonOnOff(QString band, bool on);
    void setAllButtonsOff();

    QString getCurrentButtonOn_Band();
    void setAllButtonsVisible(bool visible);
    void selectSupportedBands(const QStringList &listOfBands);

    void setMode(QString _curMmode);
    Frequency getLastFreq(const QString band, const QString mode);
    Frequency getPresetFreq(const QString band, const QString mode);
    int selectButtonGroupAndActiveBand(QString band);
signals:
void sendPresetFreq(Frequency freq);

private slots:
    void onBandSelButtonPressed(QToolButton *button);

    void onHfSelButtonPressed();
    void onVhfSelButtonPressed();
    void onMwSelButtonPressed();

private:

    QToolButton *hfSelBut = nullptr;
    QToolButton *vhfSelBut = nullptr;
    QToolButton *mwSelBut = nullptr;

    QMap<QString, QToolButton*> bandToolButList;
    QList<QString> availHfBands;
    QList<QString> availVhfBands;
    QList<QString> availMwBands;

    QMap<QString, QString> bandToButtonLabels;
    QMap<QString, QString> buttonLabelsToBand;
    QGridLayout *bandSelGridLayout;

    QList<QToolButton* > toolButList;


    QVector<QSharedPointer<BandInfo> > bands;

    QString selectedBand;
    QString curMode;


    PresetFreq presetFreqs;




    void initToolButtonTables();


    void setToolTip(QString band, QString tipTxt);


    bool checkButtonsAvailable(QString bandType);
    bool checkHfButtonsVisible();
    bool checkVhfButtonsVisible();
    bool checkMWaveButtonsVisible();

    bool isBandAvailable(QString band);
    QString convertModeForPresets(const QString mode);
    void setupButtons();
    void buildBandButtonLabels();
    QString getBandType(const QString selectedBand);
    void clearAllButtonLabels();
    void setButtonsToBandType(QString bandType);
    void removeAllButtons();
};

#endif // BANDSELBUTTONS_H
