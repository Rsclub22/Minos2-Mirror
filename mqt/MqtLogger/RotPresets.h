#ifndef ROTPRESETS_H
#define ROTPRESETS_H

#include "base_pch.h"
#include <QShortcut>
#include <QGroupBox>
#include <QShortcut>
#include "MinosLoggerEvents.h"
#include "rotatorcommon.h"
#include "rotpresetbutton.h"
#include "rigmemcommondata.h"
#include "rotpresetdialog.h"

namespace Ui {
class RotPresets;
}

class RotPresets : public QGroupBox
{
    Q_OBJECT

public:
    explicit RotPresets(QWidget *parent = nullptr);
    ~RotPresets();

    void setRotatorPresetList(QString s);
    void setContest(BaseContestLog *);

private:
    Ui::RotPresets *ui;
    QString antennaName;
    LoggerContestLog *ct = nullptr;

    QList<RotPresetButton *> presetButton;
    QVector<RotPresetData*> rotPresetData;

    QList<QShortcut *> shortCutKeyList;
    QList<QShortcut *> shiftShortCutKeyList;


    void initPresetButtons();
    void saveRotPresetButton(RotPresetData &editData);
    void setRotPresetButData(int buttonNumber, RotPresetData &editData);
    void rotPresetButtonUpdate(int buttonNumber, RotPresetData &editData);

    void traceMsg(QString msg);
signals:
    void sendRotatorPreset(QString);
    void presetTurn(QString);

private slots:
    void presetRead(int buttonNumber);
    void presetEdit(int buttonNumber);
    void presetClear(int buttonNumber);
    void presetWrite(int buttonNumber);
    void showPresetMenu(int buttonNumber);

public slots:
    void selectRotator(QString);

};

#endif // ROTPRESETS_H
