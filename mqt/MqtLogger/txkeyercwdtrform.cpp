#include <QDebug>
#include "txkeyercwdtrform.h"
//#include "ui_txkeyercwdtrform.h"
#include "cwspeedcontrol.h"
#include "txkeyerCommonConstants.h"
#include "cwentrywidget.h"
#include "keyerWidgetFactory.h"

TxKeyerCwDtrForm::TxKeyerCwDtrForm(QWidget *parent)
    : QWidget(parent)
{


    // create cw dtr keyer ui

    auto indicators = KeyerWidgetFactory::createIndicators(this);
    indicatorLayout = KeyerWidgetFactory::createRowLayout();
    indicatorLayout->addWidget(indicators.keyerIndicators);
    indicatorLayout->addWidget(indicators.pttIndicator);
    indicatorLayout->addStretch();

    auto errorMessageDisplay = KeyerWidgetFactory::createErrorMessage(this);
    keyerErrorMessageLayout = KeyerWidgetFactory::createRowLayout();
    keyerErrorMessageLayout->addWidget(errorMessageDisplay);


    auto cwSpeedSlider = KeyerWidgetFactory::createCwSpeedControl(this);
    cwSliderLayout = KeyerWidgetFactory::createRowLayout();
    cwSliderLayout->addWidget(cwSpeedSlider);

    auto cwMessageRow = KeyerWidgetFactory::createCwMessagePlayingRow(this);
    cwMessagePlayingLayout = cwMessageRow.layout;
    cwMessagePlayingLabel = cwMessageRow.label;
    cwMessagePlayingDisplay = cwMessageRow.display;

    auto cwEntry = KeyerWidgetFactory::createCwEntry(this);
    cwEntryLayout = KeyerWidgetFactory::createRowLayout();
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
