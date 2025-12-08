#include "txkeyercwdtrform.h"
#include "ui_txkeyercwdtrform.h"
#include "cwspeedcontrol.h"
#include "txkeyerCommonConstants.h"

TxKeyerCwDtrForm::TxKeyerCwDtrForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TxKeyerCwDtrForm)
{
    ui->setupUi(this);

    createCwSlider();
    ui->cwSliderLayout->addWidget(cwSpeedSlider);


}

TxKeyerCwDtrForm::~TxKeyerCwDtrForm()
{
    delete ui;
}


void TxKeyerCwDtrForm::createCwSlider()
{
    cwSpeedSlider = new CwSpeedControl(this);
    //ui->cwSpeedSliderHorizontalLayout->addWidget(cwSpeedSlider);

    cwSpeedSlider->setSpeedRange(
         TxKeyerCommon::PC_CW_KEYER_MIN_WPM,
         TxKeyerCommon::PC_CW_KEYER_MAX_WPM
        );


    connect(cwSpeedSlider, &CwSpeedControl::cwSpeedChanged, this,
            [this](int wpm){ emit sendWpmToPcCwkeyer(wpm); });




}

void TxKeyerCwDtrForm::setCwFreeTextIndicatorOnOff(bool on)
{
    if (on)
    {
        // ui->cwFreeTextPlayingIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        // ui->cwFreeTextPlayingIndicator->setToolTip(tr("CW FreeText Playing"));
    }
    else
    {
        // ui->cwFreeTextPlayingIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
        // ui->cwFreeTextPlayingIndicator->setToolTip(tr("No CW FreeText Playing"));
    }
}
