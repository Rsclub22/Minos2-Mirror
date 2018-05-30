
/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGCONTROLFRAME_H
#define RIGCONTROLFRAME_H

#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include <QShortcut>
#include "RPCCommandConstants.h"
#include "rigmemcommondata.h"


namespace Ui {
    class RigControlFrame;
}

class RigControlFrame;
class RunMemoryButton : public QObject
{
    Q_OBJECT

public:
    explicit RunMemoryButton(QToolButton *b, RigControlFrame *rcf, int no);
    ~RunMemoryButton();

    RigControlFrame *rigControlFrame;
    QToolButton* memButton;
    QMenu* memoryMenu;
    QShortcut* shortKey;
    QShortcut* shiftShortKey;
    QAction* readAction;
    QAction* writeAction;
    QAction* editAction;
    QAction* clearAction;

    int memNo;
private slots:
    void memoryUpdate();

    void memoryShortCutSelected();
    void readActionSelected();
    void editActionSelected();
    void writeActionSelected();
    void clearActionSelected();
signals:
    void clearActionSelected(int);
    void selectRadio(QString);


};

class quickBandSelData
{

public:

    QString band;
    QString freq;


};

class RigControlFrame : public QFrame
{
    Q_OBJECT

public:
    explicit RigControlFrame(QWidget *parent);
    ~RigControlFrame() override;

    Ui::RigControlFrame *ui;

    void setContest(BaseContestLog *);

    void transferDetails(memoryData::memData &m);
    void getDetails(memoryData::memData &m);
    void getRigDetails(memoryData::memData &m);


    void setRadioLoaded();
    void setRadioList(QString);
    void setBandList(QString);
    void setMode(QString);
    void setFreq(QString);
    void setRadioName(QString, QString mode);
    void setRadioState(QString);
    //void setRadioTxVertState(QString s);
    void setRadioTxVertState(bool s);
    void setRitEnableState(bool s);

    bool isRadioLoaded();


    void exitFreqEdit();
    void runButtonUpdate(int);
    void runButReadActSel(int buttonNumber);
    void runButWriteActSel(int buttonNumber);
    void runButEditActSel(int buttonNumber);


    QString getStrPassBandState(QString mode);
    int getIntPassBandState(QString mode);
    bool checkRadioState();


    void on_ContestPageChanged();



signals:
    void selectRadio(QString, QString);
    void sendRadioName(QString);
    void sendFreqControl(QString);
    void noRadioSendFreq(QString);
    void noRadioSendMode(QString);
    void sendModeToControl(QString);
    void sendRitFreq(QString);
    void ritStatus(bool);

private slots:
    void on_FontChanged();

    void freqLineEditInFocus();
    void radioBandFreq(int index);
    void noRadioSetFreq(QString);
    void noRadioSetMode(QString m);
    void freqEditSelected();
    void freqPlusShortCut_clicked(bool);
    void freqNegShortCut_clicked(bool);
    void freqPlus_ShortCut();
    void freqNeg_ShortCut();
    void changeMainRadioFreq();
    void changeRitRadioFreq(QString);
    void on_radioNameSel_activated(const QString &arg1);


    void exitRitFreqEdit();
    void freqRitEditSelected();
    void ritButtonSelected();
public slots:


    void returnChangeRadioFreq();

    void runButClearActSel(int buttonNumber);


private:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;


    // memory buttons
    memoryData::memData getRunMemoryData(int memoryNumber);

    void setRunMemoryData(int memoryNumber, memoryData::memData m);

    LoggerContestLog *ct;

    QMap<int, RunMemoryButton *> runButtonMap;
    QVector<quickBandSelData> listOfBands;



    QShortcut* freqEditShortKey;

    QShortcut *freqPlusShortCut;
    QShortcut *freqNegShortCut;

    bool radioLoaded;
    bool radioConnected;
    bool radioError;
    bool freqEditOn;
    QString curFreq;
    QString curMode;

    bool ritEnable;
    bool ritOn;
    QString curRit;

    QStringList listOfRadios;
    QString radioName;
    QString rigAppName;
    QString radioState;
    //QStringList listOfBands;


    QString lastFreq;

    void sendModeToRadio(QString);
    void freqLineEditBkgnd(bool status);
    void freqLineEditFrameColour(bool status);

    void ritFreqLineEditFrameColour(bool status);

    void initRigFrame(QWidget *parent);
    void initPassBandRadioButtons();
    void noRadioSendOutFreq(QString f);
    void noRadioSendOutMode(QString m);

    void initRunMemoryButton();
    void loadRunButtonLabels();

    void traceMsg(QString msg);

    QString extractKhz(QString f);
    void loadMemories();
    void mgmLabelVisible(bool state);
    bool checkValidFreq(QString freq);
    void sendFreq(QString f);
    QString calcNewFreq(double incFreq);

    int calcMinosMode(QString mode);

    void freqPlusMinusButton(double freq);


    void showRitButOn();
    void showRitButOff();

};


class FreqLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    FreqLineEdit(QWidget *parent);
    ~FreqLineEdit();
    void changeFreq(bool direction);
    //QString convertFreqString(double frequency);

signals:
    void receivedFocus() ;
    void lostFocus();
    void newFreq();
    void freqEditReturn();
private:


    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

} ;



class RitLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    RitLineEdit(QWidget *parent);
    ~RitLineEdit();
    void changeFreq(bool direction);

signals:
    void receivedFocus() ;
    void lostFocus();
    void newFreq(QString);
    void freqEditReturn();
private:


    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);


} ;


#endif // RIGCONTROLFRAME_H
