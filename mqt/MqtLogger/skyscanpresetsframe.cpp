#include "skyscanpresetsframe.h"
#include "ui_skyscanpresetsframe.h"
#include "rotatorcommon.h"
#include "MTrace.h"

skyScanPresetsFrame::skyScanPresetsFrame(QWidget *parent)
    : MinosPanel(parent)
    , ui(new Ui::skyScanPresetsFrame)
{
    ui->setupUi(this);

    initPresetButtons();

    ui->skyScanPresetsGroupBox->setEnabled(false);
}

skyScanPresetsFrame::~skyScanPresetsFrame()
{
    delete ui;
}




void skyScanPresetsFrame::initPresetButtons()
{
    skyScanPresetButton.clear();

    QList<QToolButton*> ui_presetbuttons;
    ui_presetbuttons << ui->skyScanPresetTb0 << ui->skyScanPresetTb1 << ui->skyScanPresetTb2
                     << ui->skyScanPresetTb3 << ui->skyScanPresetTb4 << ui->skyScanPresetTb5;


    QStringList buttonLabels;
    for (unsigned int i = 0; i < sizeof(RotPresetData::presetButtonLabels)/sizeof(const char *); i++)
    {
        buttonLabels.append(RotPresetData::tr(RotPresetData::presetButtonLabels[i]));
    }

    for (int i = 0; i < ui_presetbuttons.count(); i++)
    {

        // build array of buttons
        skyScanPresetButton.append(QSharedPointer<PresetButton>::create(ui_presetbuttons[i], i, nullptr, nullptr, buttonLabels));
        skyScanPresetButton[i]->disableMenu();


        // only using click button
        connect(skyScanPresetButton[i].data(), &PresetButton::presetReadAction, this, [this, i]() {presetRead(i);});


        //connect(skyScanPresetButton[i], &PresetButton::presetShortCutRecall, this, [this, i]() {presetRead(i);});
        //connect(presetButton[i], &PresetButton::presetShiftShortCutRecall, this, [this, i]() {showPresetMenu(i);});

        //connect(presetButton[i], &PresetButton::presetEditAction, this, [this, i]() {presetEdit(i);});
        //connect(presetButton[i], &PresetButton::presetWriteAction, this, [this, i]() {presetWrite(i);});
        //connect(presetButton[i], &PresetButton::presetClearAction, this, [this, i]() {presetClear(i);});

    }
}


void skyScanPresetsFrame::presetRead(int buttonNumber)
{
    traceMessage(QString("preset button number %1 pressed").arg(buttonNumber));


    if (buttonNumber >=0 && buttonNumber < skyScanPresetButton.count())
    {
        if (!skyScanPresetButton[buttonNumber]->getText().isEmpty())
        {
            emit recallSkyScanPreset(buttonNumber);
        }
    }
}

void skyScanPresetsFrame::setPresetList(QString skyScanPresetList)
{

    traceMessage(QString("preset name list received from rotator control = %1").arg(skyScanPresetList));

    if (!skyScanPresetList.isEmpty() && skyScanPresetList.contains(':'))
    {

        QStringList presets = skyScanPresetList.split(':');
        if (presets.count() > 0)
        {
            for (int i = 0; i < presets.count(); i++)
            {
                QStringList p = presets[i].split(',');
                if (p.count() == 2)
                {
                    skyScanPresetButton[i]->setText(QString("%1:%2").arg(i + 1).arg(p[1]));

                }

            }
        }


    }
}

void skyScanPresetsFrame::setSkyCanVisible(bool visible)
{
    traceMessage(QString("Set SkyScanVisible = %1").arg(visible ? "True" : "False"));

    ui->skyScanPresetsGroupBox->setEnabled(visible);

}


void skyScanPresetsFrame::traceMessage(QString msg)
{
    trace(QString("[skyScanPresets]%1").arg(msg));
}

