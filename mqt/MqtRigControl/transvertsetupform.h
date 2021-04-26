#ifndef TRANSVERTSETUPFORM_H
#define TRANSVERTSETUPFORM_H

#include <QWidget>
#include "ui_transvertsetupform.h"
#include "BandList.h"
#include "focuswatcher.h"
#include "rigcommon.h"



namespace Ui {
class transVertSetupForm;
}





class TransVertSetupForm : public QWidget
{
    Q_OBJECT
public:
    explicit TransVertSetupForm(QSharedPointer<scatParams> _radioData, const QString _bandName, const QVector<QSharedPointer<BandInfo> > _bands, QWidget *parent = nullptr);


    bool transVertValueChanged = false;
    bool transVertNameChanged = false;


    bool transVertOffsetOk = false;

    void setBand(QString b);
    QString getBand();

    //void antSwNumVisible(bool visible);
    QString getTransVerSwNum();
    void setTransVerSwNum(QString s);

    void setUiItemsVisible(bool visible);
    void loadBandFreqLimits();
    void setRadioFreqBox(Frequency f);
    void setTargetFreqBox(Frequency f);
    void setOffsetFreqLabel(Frequency f);

    void setLocTVSwComport(QString p);
    void setEnableTransVertSwBoxVisible(bool visible);

    //QSharedPointer<TransVertParams> getTransVertData(){return transVertData;}
    //void setTransVertData(QSharedPointer<TransVertParams> tvp){transVertData = tvp;}
private:

    Ui::transVertSetupForm *ui;
    QSharedPointer<scatParams> radioData;
    QString bandName;
    const QVector<QSharedPointer<BandInfo> > bands;

    FocusWatcher *radioFreqEdit;
    FocusWatcher *targetFreqEdit;

    bool radioFreqOK = false;
    bool targetFreqOK = false;

    void loadBandSel();
    bool freqInBand(Frequency f, QString band);
private slots:
    //void bandSelected();
    void calcOffset();

    void transVertSwNumSel();
public slots:
    void radioFreqEditfocusChange(QObject *, bool, QFocusEvent *event);
    void targetFreqEditfocusChange(QObject *, bool, QFocusEvent *event);
};

#endif // TRANSVERTSETUPFORM_H
