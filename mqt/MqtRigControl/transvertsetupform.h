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
    explicit TransVertSetupForm(TransVertParams* _transvertData, QWidget *parent = nullptr);


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

private:

    Ui::transVertSetupForm *ui;
    TransVertParams *transVertData;

    FocusWatcher *radioFreqEdit;
    FocusWatcher *targetFreqEdit;

    bool radioFreqOK = false;
    bool targetFreqOK = false;

    void loadBandSel();
private slots:
    //void bandSelected();
    void calcOffset();

    void transVertSwNumSel();
public slots:
    void radioFreqEditfocusChange(QObject *, bool, QFocusEvent *event);
    void targetFreqEditfocusChange(QObject *, bool, QFocusEvent *event);
};

#endif // TRANSVERTSETUPFORM_H
