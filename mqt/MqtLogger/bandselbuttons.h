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
    const QString BUTTON_ON_STYLE = QString("background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
    const QString BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
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
    explicit BandSelButtons(const QVector<QSharedPointer<BandInfo> > &bands, const QList<QToolButton *> &bandButtons, const BandSelTabWidget &_bandSelTabWidget, QObject *parent = nullptr);



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
signals:
void sendPresetFreq(Frequency freq);

private slots:
    void onBandSelButtonPressed(QString key);

private:

    QMap<QString, QToolButton*> bandToolButList;
    QList<QString> activeBands;

    BandSelTabWidget bandSelTabWidget;

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
    void removeBandSelTab(QString tabLabel);
    bool isBandAvailable(QString band);
    QString selectedBandType(const QString selectedBand);
    void setTabToCurrentBandType(QString selectedBand);
    QString convertModeForPresets(const QString mode);
};

#endif // BANDSELBUTTONS_H
