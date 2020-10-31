#ifndef TXVMBUTTONSFRAME_H
#define TXVMBUTTONSFRAME_H


#include "base_pch.h"
#include <QShortcut>
#include <QGroupBox>
#include <QObject>
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"
#include "txvmbuttondialog.h"

namespace Ui {
class TxVmButtonsFrame;
}

class TxVmButtonsFrame;

class TxVoiceMemButton: public QObject
{


    Q_OBJECT
public:
    explicit TxVoiceMemButton(QToolButton *b, TxVmButtonsFrame *vmf, int no);
    ~TxVoiceMemButton();


    TxVmButtonsFrame *txVmButtonsFrame;
    QToolButton* vmButton;
    QMenu* vmMenu;
    QShortcut* shortKey;
    QShortcut* shiftShortKey;
    QAction* readAction;
    QAction* newAction;
    QAction* editAction;
    QAction* setupAction;


    void showButtonOnOff(bool state);

private slots:
    void memoryShortCutSelected();
    void readActionSelected();
    void editActionSelected();
    void writeActionSelected();

    void buttonSelected();

    void onSetupActionSelected();
signals:

    void buttonActivated(int);


private:
    int memNo;

};

class TxVmButtonsFrame : public QGroupBox
{
    Q_OBJECT

public:
    explicit TxVmButtonsFrame(QWidget *parent = nullptr);
    ~TxVmButtonsFrame();

    QMap<int, TxVoiceMemButton*> txVmButtonMap;





    void readActionSel(int buttonNumber);
    void readActionSelected(int buttonNumber);
    void editActionSelected(int buttonNumber);
    void writeActionSelected(int buttonNumber);
    void buttonUpdate(int buttonNumber);

private:
    Ui::TxVmButtonsFrame *ui;


    VoiceKeyerBase* txVoiceKeyer;
    VoiceKeyerFactory* voiceKeyerFactory;


    int runButtonOnNum;

    QList<QToolButton*> voiceMemButtonList;

    QList<VoiceKeyerParams> vmKeyParamList;

    void initRunMemoryButton();
    void loadRunButtonLabels();

    void initTxVmButton();

    void setRunButtonText(const int buttonNumber, const QString name);
private slots:
    void radioIsConnected(bool on);
    void onVoiceKeyerSelect(int idx);
};

#endif // TXVMBUTTONSFRAME_H
