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
    initPresetButtons();
}

RotPresets::~RotPresets()
{
    delete ui;
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

        rotPresetData.clear();

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

    for (int i = 0; i < ui_presetbuttons.count(); i++)
    {

        presetButton.append(new RotPresetButton(ui_presetbuttons[i], i));

        connect(presetButton[i], &RotPresetButton::presetReadAction, [this, i]() {presetRead(i);});
        connect(presetButton[i], &RotPresetButton::presetEditAction, [this, i]() {presetEdit(i);});
        connect(presetButton[i], &RotPresetButton::presetWriteAction, [this, i]() {presetWrite(i);});
        connect(presetButton[i], &RotPresetButton::presetClearAction, [this, i]() {presetClear(i);});


    }

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
        RotPresetDialog presetDialog(buttonNumber, &editData, &curData, this);


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
        RotPresetDialog presetDialog(buttonNumber, &editData, &curData, this);


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
    presetButton[buttonNumber]->presetButton->setText(QString("%1: %2").arg(QString::number(buttonNumber + 1)).arg(editData.name) );
    QString tTipStr = "Bearing = " + editData.bearing;
    presetButton[buttonNumber]->presetButton->setToolTip(tTipStr);
}

void RotPresets::saveRotPresetButton(RotPresetData& editData)
{
    QString msg;
    msg = QString("%1:%2:%3").arg(QString::number(editData.number)).arg(editData.name).arg(editData.bearing);
    emit sendRotatorPreset(msg);
}




