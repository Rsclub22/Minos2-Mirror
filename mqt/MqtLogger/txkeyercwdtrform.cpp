#include "txkeyercwdtrform.h"
#include "ui_txkeyercwdtrform.h"
#include "cwspeedcontrol.h"
#include "txkeyerCommonConstants.h"
#include "cwentrywidget.h"

TxKeyerCwDtrForm::TxKeyerCwDtrForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TxKeyerCwDtrForm)
{
    ui->setupUi(this);

    createCwSlider();
    ui->cwSliderLayout->addWidget(cwSpeedSlider);

    connect(ui->cwEntry, &CwEntryWidget::cwEntryReturnPressed, this,
             [this](){ emit cwEntryReturnPressed(); });
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

void TxKeyerCwDtrForm::setCwSliderValue(int value)
{
    cwSpeedSlider->setValue(value);
}


QString TxKeyerCwDtrForm::getCwEntryText()
{
    return ui->cwEntry->getCwEntryText();
}

void TxKeyerCwDtrForm::setCwFreeTextIndicatorOnOff(bool on)
{
    ui->cwEntry->setCwFreeTextIndicatorOnOff(on);
}


void TxKeyerCwDtrForm::setKeyerAvailableIndicatorOnOff(bool on)
{
    ui->KeyIndicators->setKeyerAvailableIndicatorOnOff(on);
}

void TxKeyerCwDtrForm::setRepeatIndicatorOnOff(bool on)
{
    ui->KeyIndicators->setRepeatIndicatorOnOff(on);
}

void TxKeyerCwDtrForm::setEOMLabelText(const QString text)
{
    ui->KeyIndicators->setEOMLabelText(text);
}

void TxKeyerCwDtrForm::clearEOMLabelText()
{
    ui->KeyIndicators->clearEOMLabelText();
}

void TxKeyerCwDtrForm::setPttEnabledIndicatorOnOff(const bool on)
{
    ui->PttIndicators->setPttEnabledIndicator(on);
}


void TxKeyerCwDtrForm::setTxStatusIndicatorOnOff(const bool on)
{
    ui->PttIndicators->setTxStatusIndicator(on);
}

void TxKeyerCwDtrForm::setPttTypeText(const QString text)
{
    ui->PttIndicators->setPttTypeText(text);
}

void TxKeyerCwDtrForm::clearPttTypeText()
{
    ui->PttIndicators->clearPttTypeText();
}

void TxKeyerCwDtrForm::setStoredMessagePlayingDisplay(const QString msg)
{
    ui->storedMessagePlayingDisplay->setText(msg);
}

void TxKeyerCwDtrForm::clearStoredMessagePlayingDisplay()
{
    ui->storedMessagePlayingDisplay->clear();
}

void TxKeyerCwDtrForm::setErrorMessageDisplayText(const QString errormsg)
{
    ui->keyerErrorMessageDisplay->setErrorMessage(errormsg);
}

void TxKeyerCwDtrForm::clearErrorMessageDisplayText()
{
    ui->keyerErrorMessageDisplay->clearErrorMessage();
}


void TxKeyerCwDtrForm::showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour)
{
    ui->keyerErrorMessageDisplay->showTemporaryErrorMessage(msg, timeoutMs, colour);
}
