#ifndef BANDSELBUTTONS_H
#define BANDSELBUTTONS_H

#include <QObject>
#include <QToolButton>
#include <QRadioButton>
#include <QMap>
#include "BandList.h"
#include "rigcommon.h"
#include "radiosettingdialog.h"




namespace bandSelButtonData
{
    const QString BUTTON_ON_STYLE = QString("background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black");
    const QString BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black");
    const QString BUTTON_NOT_CONTEST_STYLE = QString("background-color: Yellow ; border-style: outset; border-width: 1px; border-color: black");
    const QString TYPE_BUTTON_ON_STYLE = QString("background-color: White ; border-style: outset; border-width: 1px; border-color: black");
    const QString TYPE_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black");
}


class BandSelButtons : public QObject
{
     Q_OBJECT

public:
    explicit BandSelButtons(const QVector<QSharedPointer<BandInfo> > &bands,  QGridLayout *_bandSelGrid, QObject *parent = nullptr);

    void setContest(BaseContestLog *contest);

    void setButtonOnOff(QToolButton *t, bool on);
    void setAllButtonsOff();

    void setAllButtonsVisible(bool visible);
    void selectSupportedBands(const QStringList &listOfBands);

    void setMode(QString _curMmode);
    Frequency getLastFreq(const QString band, const QString mode);
    Frequency getPresetFreq(const QString band, const QString mode);
    int selectButtonGroupAndActiveBand(const QString band);
    int selectButtonGroupAndActiveBand(const Frequency &freq);
    void setContestBand(QString contestBand_);
    bool findBand(const Frequency &freq, QVector<QSharedPointer<BandInfo> > &bands, QString &foundBand);
    QToolButton *findToolButton(QString band);

    void setPreviousFreq(QString mode, Frequency freq);
    void setPrevFreqToolTip(QString mode);

    void readPresetFreqsFromIni(const QVector<QSharedPointer<BandInfo> > bands);
signals:
    void sendPresetFreq(Frequency freq);
    void sendBandChange(QString band);

private slots:
    void onBandSelButtonPressed(QToolButton *button);
private:

    QList<QString> availHfBands;

    QGridLayout *bandSelGridLayout;

    QList<QToolButton* > toolButList;

    BaseContestLog *ct = nullptr;
    QVector<QSharedPointer<BandInfo> > bands;

    QString selectedBand;
    QString contestBand;
    QString curMode;

    PresetFreq presetFreqs;

    void initToolButtonTables();

    void setToolTip(QString band, QString tipTxt);

    bool checkButtonsAvailable(QString bandType);

    bool isBandAvailable(QString band);
    QString convertModeForPresets(const QString mode);
    void setupButtons();
    QString getBandType(const QString selectedBand);
    void clearAllButtonLabels();
    void setButtonsToBandType();
    void removeAllButtons();
};

#endif // BANDSELBUTTONS_H
