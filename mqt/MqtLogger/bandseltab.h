#ifndef BANDSELTAB_H
#define BANDSELTAB_H

#include <QTabWidget>
#include <QToolButton>
#include <QMap>
#include "BandList.h"




namespace bandSelData
{
    const QString BUTTON_ON_STYLE = QString("background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
    const QString BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
}


class BandSelTab : public QTabWidget
{
     Q_OBJECT

public:
    explicit BandSelTab(QWidget *parent = nullptr);



    void setButtonVisible(QString band, bool visible);
    void setHf(bool state);

    int setButtonOnOff(QString band, bool on);
    void setAllButtonsOff();

    QString getCurrentButtonOn_Band();
    void setAllButtonsVisible(bool visible);
    void selectSupportedBands(const QStringList &listOfBands);
private slots:
    void onBandSelButtonPressed(QString key);

private:

    QMap<QString, QToolButton*> bandToolButList;

    QVector<QSharedPointer<BandInfo> > bands;

    QString selectedBand;



    void initToolButtonTables();

};

#endif // BANDSELTAB_H
