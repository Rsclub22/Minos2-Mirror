#include "base_pch.h"
#include "ContestApp.h"
#include "LoggerContest.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "qsologframe.h"
#include "SendRPCDM.h"
#include "rotatorcommon.h"

#include "RotPresets.h"
#include "ui_RotPresets.h"




RotPresets::RotPresets(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::RotPresets)
{
    ui->setupUi(this);

    for (int i = 0; i < presetShortCutKeys.count(); i++)
    {
        shortCutKeyList.append(new QShortcut(QKeySequence(presetShortCutKeys[i]), parent));
    }

    for (int i = 0; i < presetMenuShortCutKeys.count(); i++)
    {
        shiftShortCutKeyList.append(new QShortcut(QKeySequence(presetMenuShortCutKeys[i]), parent));
    }

    initPresetButtons();
}

RotPresets::~RotPresets()
{
    delete ui;
    for(auto const &b: presetButton)
    {
        delete b;
    }
    clearPresetData();
}
void RotPresets::clearPresetData()
{
    for(auto const &b: rotPresetData)
    {
        delete b;
    }
    rotPresetData.clear();
}
void RotPresets::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);
}
void RotPresets::selectRotator(QString s)
{
    antennaName = s;
}
void RotPresets::traceMsg(QString msg)
{
    trace(QString("RotPresets: %1 - %2").arg(antennaName).arg( msg));
}
void RotPresets::setRotatorPresetList(QString s)
{
    if (!s.isEmpty() && s.contains(':'))
    {

        clearPresetData();

        QStringList presets = s.split(':');
        for (int i = 0; i < presets.count(); i++)
        {
            QStringList p = presets[i].split(',');
            rotPresetData.append(new RotPresetData(p[0].toInt(), p[1], p[2]));
            rotPresetButtonUpdate(i, *rotPresetData[i]);
        }

    }

}

/**************************** Quick Preset Buttons **************************/


void RotPresets::initPresetButtons()
{

    QList<QToolButton*> ui_presetbuttons;
    ui_presetbuttons << ui->presetButton0 << ui->presetButton1 << ui->presetButton2 << ui->presetButton3 << ui->presetButton4
                     << ui->presetButton5 << ui->presetButton6 << ui->presetButton7 << ui->presetButton8 << ui->presetButton9;

    QStringList buttonLabels;
    for (unsigned int i = 0; i < sizeof(RotPresetData::presetButtonLabels)/sizeof(const char *); i++)
    {
        buttonLabels.append(RotPresetData::tr(RotPresetData::presetButtonLabels[i]));
    }

    for (int i = 0; i < ui_presetbuttons.count(); i++)
    {

        // build array of buttons
        presetButton.append(new PresetButton(ui_presetbuttons[i], i, shortCutKeyList[i], shiftShortCutKeyList[i], buttonLabels));

        connect(presetButton[i], &PresetButton::presetShortCutRecall, [this, i]() {presetRead(i);});
        connect(presetButton[i], &PresetButton::presetShiftShortCutRecall, [this, i]() {showPresetMenu(i);});
        connect(presetButton[i], &PresetButton::presetReadAction, [this, i]() {presetRead(i);});
        connect(presetButton[i], &PresetButton::presetEditAction, [this, i]() {presetEdit(i);});
        connect(presetButton[i], &PresetButton::presetWriteAction, [this, i]() {presetWrite(i);});
        connect(presetButton[i], &PresetButton::presetClearAction, [this, i]() {presetClear(i);});

    }
}




void RotPresets::showPresetMenu(int buttonNumber)
{
    presetButton[buttonNumber]->showButtonMenu();
}



void RotPresets::presetRead(int buttonNumber)
{
    if (!rotPresetData.isEmpty()  && buttonNumber < rotPresetData.count())
    {
        emit presetTurn(rotPresetData[buttonNumber]->bearing);
    }
}

void RotPresets::presetEdit(int buttonNumber)
{


    if (!rotPresetData.isEmpty()  && buttonNumber < rotPresetData.count())
    {
        RotPresetData editData(buttonNumber, rotPresetData[buttonNumber]->name, rotPresetData[buttonNumber]->bearing);
        RotPresetData curData(buttonNumber, rotPresetData[buttonNumber]->name, rotPresetData[buttonNumber]->bearing);

        traceMsg(QString("RotFrame: Preset Edit Selected = %1").arg(QString::number(buttonNumber + 1)));
        RotPresetDialog presetDialog(this, buttonNumber, &editData, &curData, tr("Edit"));


        if (presetDialog.exec() == QDialog::Accepted)
        {
            if (editData.name != curData.name || editData.bearing != curData.bearing)
            {
                setRotPresetButData(buttonNumber, editData);
                rotPresetButtonUpdate(buttonNumber, editData);
            }

        }
    }


}

void RotPresets::presetClear(int buttonNumber)
{
    traceMsg(QString("RotFrame: Preset Clear Selected = %1").arg(QString::number(buttonNumber +1)));
    if (!rotPresetData.isEmpty() && buttonNumber < rotPresetData.count())
    {
        // clear this preset
        RotPresetData pData(0, "", "0");
        rotPresetButtonUpdate(buttonNumber, pData);
        rotPresetButtonUpdate(buttonNumber, pData);
    }
}

//void RotPresets::presetButtonUpdate(int buttonNumber)
//{

//}

void RotPresets::presetWrite(int buttonNumber)
{
    traceMsg(QString("RotFrame: Preset Write Selected = %1").arg(QString::number(buttonNumber +1)));
    if (!rotPresetData.isEmpty()  && buttonNumber < rotPresetData.count())
    {
        RotPresetData editData(buttonNumber, "", "0");
        RotPresetData curData(buttonNumber, "", "0");

        traceMsg(QString("RotFrame: Preset Edit Selected = %1").arg(QString::number(buttonNumber + 1)));
        RotPresetDialog presetDialog(this, buttonNumber, &editData, &curData, tr("New"));


        if (presetDialog.exec() == QDialog::Accepted)
        {
            if (editData.name != curData.name || editData.bearing != curData.bearing)
            {
                setRotPresetButData(buttonNumber, editData);
                rotPresetButtonUpdate(buttonNumber, editData);
            }

        }
    }
}


void RotPresets::setRotPresetButData(int buttonNumber, RotPresetData& editData)
{
    rotPresetData[buttonNumber]->name = editData.name;
    rotPresetData[buttonNumber]->bearing = editData.bearing;
    saveRotPresetButton(editData);
}


void RotPresets::rotPresetButtonUpdate(int buttonNumber, RotPresetData& editData)
{
    presetButton[buttonNumber]->presetButton->setText(QString("%1: %2\r\n%3").arg(QString::number(buttonNumber + 1)).arg(editData.name).arg(editData.bearing) );
    QString tTipStr = tr("Bearing = %1").arg(editData.bearing);
    presetButton[buttonNumber]->presetButton->setToolTip(tTipStr);
}

void RotPresets::saveRotPresetButton(RotPresetData& editData)
{
    QString msg;
    msg = QString("%1:%2:%3").arg(QString::number(editData.number)).arg(editData.name).arg(editData.bearing);
    emit sendRotatorPreset(msg);
}




