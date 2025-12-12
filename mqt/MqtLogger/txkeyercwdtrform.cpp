#include <QDebug>
#include "txkeyercwdtrform.h"
#include "ui_txkeyercwdtrform.h"
#include "cwspeedcontrol.h"
#include "txkeyerCommonConstants.h"
#include "cwentrywidget.h"

TxKeyerCwDtrForm::TxKeyerCwDtrForm(QWidget *parent)
    : QWidget(parent)
//    , ui(new Ui::TxKeyerCwDtrForm)
{
//    ui->setupUi(this);

    // create cw dtr keyer ui

    indicatorLayout = new QHBoxLayout();
    indicatorLayout->setContentsMargins(2, 0, 0, 2);
    indicatorLayout->setSpacing(6);

    keyerIndicatorsWidget = new KeyerIndicatorsWidget();
    pttIndicatorWidget = new PttIndicatorWidget();

    keyerIndicatorsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    keyerIndicatorsWidget->setMaximumHeight(keyerIndicatorsWidget->sizeHint().height());

    pttIndicatorWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    pttIndicatorWidget->setMaximumHeight(pttIndicatorWidget->sizeHint().height());


    indicatorLayout->addWidget(keyerIndicatorsWidget);
    indicatorLayout->addWidget(pttIndicatorWidget);
    indicatorLayout->addStretch();

    keyerErrorMessageLayout = new QHBoxLayout();
    keyerErrorMessageLayout->setContentsMargins(2, 0, 0, 2);
    keyerErrorMessageLayout->setSpacing(6);

    keyerErrorMessageDisplay = new KeyerErrorMessageWidget();
    keyerErrorMessageDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    keyerErrorMessageDisplay->setMaximumHeight(keyerErrorMessageDisplay->sizeHint().height());

    keyerErrorMessageLayout->addWidget(keyerErrorMessageDisplay);

    cwSliderLayout = new QHBoxLayout();
    cwSpeedSlider = new CwSpeedControl();
    cwSpeedSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    cwSpeedSlider->setMaximumHeight(cwSpeedSlider->sizeHint().height());

    cwSliderLayout->addWidget(cwSpeedSlider);

    cwMessagePlayingLayout = new QHBoxLayout();
    cwMessagePlayingLabel = new QLabel(tr("Stored Message Playing: "));
    cwMessagePlayingLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    cwMessagePlayingLabel->setMaximumHeight(cwMessagePlayingLabel->sizeHint().height());

    cwMessagePlayingDisplay = new QLabel();
    cwMessagePlayingDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    cwMessagePlayingDisplay->setMaximumHeight(cwMessagePlayingDisplay->sizeHint().height());

    cwMessagePlayingLayout->addWidget(cwMessagePlayingLabel);
    cwMessagePlayingLayout->addWidget(cwMessagePlayingDisplay);

    cwEntryLayout = new QHBoxLayout();
    cwEntry = new CwEntryWidget();
    cwEntry->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    cwEntry->setMaximumHeight(cwEntry->sizeHint().height());

    cwEntryLayout->addWidget(cwEntry);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);
    mainLayout->addLayout(indicatorLayout);
    mainLayout->addLayout(keyerErrorMessageLayout);
    mainLayout->addLayout(cwSliderLayout);
    mainLayout->addLayout(cwMessagePlayingLayout);
    mainLayout->addLayout(cwEntryLayout);

    setLayout(mainLayout);



    connect(cwEntry, &CwEntryWidget::cwEntryReturnPressed, this,
             [this](){ emit cwEntryReturnPressed(); });

    connect(cwSpeedSlider, &CwSpeedControl::cwSpeedChanged, this,
            [this](int wpm){ emit sendWpmToPcCwkeyer(wpm); });

    qDebug() << "Form:" << this->objectName()
             << "sizeHint =" << this->sizeHint()
             << "minSizeHint =" << this->minimumSizeHint();

}

TxKeyerCwDtrForm::~TxKeyerCwDtrForm()
{

}



void TxKeyerCwDtrForm::setCwSliderValue(int value)
{
    cwSpeedSlider->setValue(value);
}


QString TxKeyerCwDtrForm::getCwEntryText()
{
    return cwEntry->getCwEntryText();
}

void TxKeyerCwDtrForm::setCwFreeTextIndicatorOnOff(bool on)
{
    cwEntry->setCwFreeTextIndicatorOnOff(on);
}


void TxKeyerCwDtrForm::setKeyerAvailableIndicatorOnOff(bool on)
{
    keyerIndicatorsWidget->setKeyerAvailableIndicatorOnOff(on);
}

void TxKeyerCwDtrForm::setRepeatIndicatorOnOff(bool on)
{
    keyerIndicatorsWidget->setRepeatIndicatorOnOff(on);
}

void TxKeyerCwDtrForm::setEOMLabelText(const QString text)
{
    keyerIndicatorsWidget->setEOMLabelText(text);
}

void TxKeyerCwDtrForm::clearEOMLabelText()
{
    keyerIndicatorsWidget->clearEOMLabelText();
}

void TxKeyerCwDtrForm::setPttEnabledIndicatorOnOff(const bool on)
{
    pttIndicatorWidget->setPttEnabledIndicator(on);
}


void TxKeyerCwDtrForm::setTxStatusIndicatorOnOff(const bool on)
{
    pttIndicatorWidget->setTxStatusIndicator(on);
}

void TxKeyerCwDtrForm::setPttTypeText(const QString text)
{
    pttIndicatorWidget->setPttTypeText(text);
}

void TxKeyerCwDtrForm::clearPttTypeText()
{
    pttIndicatorWidget->clearPttTypeText();
}

void TxKeyerCwDtrForm::setStoredMessagePlayingDisplay(const QString msg)
{
    cwMessagePlayingDisplay->setText(msg);
}

void TxKeyerCwDtrForm::clearStoredMessagePlayingDisplay()
{
    cwMessagePlayingDisplay->clear();
}

void TxKeyerCwDtrForm::setErrorMessageDisplayText(const QString errormsg)
{
    keyerErrorMessageDisplay->setErrorMessage(errormsg);
}

void TxKeyerCwDtrForm::clearErrorMessageDisplayText()
{
    keyerErrorMessageDisplay->clearErrorMessage();
}


void TxKeyerCwDtrForm::showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour)
{
    keyerErrorMessageDisplay->showTemporaryErrorMessage(msg, timeoutMs, colour);
}
