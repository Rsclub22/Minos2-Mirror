#include <QDebug>
#include "txkeyercwrigcontrolform.h"
#include "ui_txkeyercwrigcontrolform.h"
#include "txkeyerCommonConstants.h"

TxKeyerCwRigControlForm::TxKeyerCwRigControlForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TxKeyerCwRigControlForm)
{
    ui->setupUi(this);

    createCwSlider();
    ui->cwSliderLayout->addWidget(cwSpeedSlider);

    connect(ui->cwEntry, &CwEntryWidget::cwEntryReturnPressed, this,
            [this](){ emit cwEntryReturnPressed(); });

    qDebug() << "Form:" << this->objectName()
             << "sizeHint =" << this->sizeHint()
             << "minSizeHint =" << this->minimumSizeHint();

}

TxKeyerCwRigControlForm::~TxKeyerCwRigControlForm()
{
    delete ui;
}


void TxKeyerCwRigControlForm::createCwSlider()
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

void TxKeyerCwRigControlForm::setCwSliderValue(int value)
{
    cwSpeedSlider->setValue(value);
}


QString TxKeyerCwRigControlForm::getCwEntryText()
{
    return ui->cwEntry->getCwEntryText();
}

void TxKeyerCwRigControlForm::setCwFreeTextIndicatorOnOff(bool on)
{
    ui->cwEntry->setCwFreeTextIndicatorOnOff(on);
}


void TxKeyerCwRigControlForm::setKeyerAvailableIndicatorOnOff(bool on)
{
    ui->KeyIndicators->setKeyerAvailableIndicatorOnOff(on);
}

void TxKeyerCwRigControlForm::setRepeatIndicatorOnOff(bool on)
{
    ui->KeyIndicators->setRepeatIndicatorOnOff(on);
}

void TxKeyerCwRigControlForm::setEOMLabelText(const QString text)
{
    ui->KeyIndicators->setEOMLabelText(text);
}

void TxKeyerCwRigControlForm::clearEOMLabelText()
{
    ui->KeyIndicators->clearEOMLabelText();
}

void TxKeyerCwRigControlForm::setPttEnabledIndicatorOnOff(const bool on)
{
    ui->PttIndicators->setPttEnabledIndicator(on);
}


void TxKeyerCwRigControlForm::setTxStatusIndicatorOnOff(const bool on)
{
    ui->PttIndicators->setTxStatusIndicator(on);
}

void TxKeyerCwRigControlForm::setPttTypeText(const QString text)
{
    ui->PttIndicators->setPttTypeText(text);
}

void TxKeyerCwRigControlForm::clearPttTypeText()
{
    ui->PttIndicators->clearPttTypeText();
}

void TxKeyerCwRigControlForm::setStoredMessagePlayingDisplay(const QString msg)
{
    ui->storedMessagePlayingDisplay->setText(msg);
}

void TxKeyerCwRigControlForm::clearStoredMessagePlayingDisplay()
{
    ui->storedMessagePlayingDisplay->clear();
}

void TxKeyerCwRigControlForm::setErrorMessageDisplayText(const QString errormsg)
{
    ui->keyerErrorMessageDisplay->setErrorMessage(errormsg);
}

void TxKeyerCwRigControlForm::clearErrorMessageDisplayText()
{
    ui->keyerErrorMessageDisplay->clearErrorMessage();
}


void TxKeyerCwRigControlForm::showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour)
{
    ui->keyerErrorMessageDisplay->showTemporaryErrorMessage(msg, timeoutMs, colour);
}
