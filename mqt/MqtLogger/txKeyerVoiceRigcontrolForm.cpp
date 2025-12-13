#include <QDebug>
#include "txKeyerVoiceRigcontrolForm.h"
#include "keyerWidgetFactory.h"


TxVoiceRigControlForm::TxVoiceRigControlForm(QWidget *parent)
    : QWidget(parent)
{

    // create ui

    indicators = KeyerWidgetFactory::createIndicators(this);
    indicatorLayout = KeyerWidgetFactory::createRowLayout(this);
    indicatorLayout->addWidget(indicators.keyerIndicators);
    indicatorLayout->addWidget(indicators.pttIndicator);
    indicatorLayout->addStretch();


    //keyerErrorMessageDisplay = KeyerWidgetFactory::createErrorMessage(this);
    //keyerErrorMessageLayout = KeyerWidgetFactory::createRowLayout(this);
    //keyerErrorMessageLayout->addWidget(keyerErrorMessageDisplay);



    QVBoxLayout *mainLayout = KeyerWidgetFactory::createMainLayout(this);
    mainLayout->addLayout(indicatorLayout);
    //mainLayout->addLayout(keyerErrorMessageLayout);

    setLayout(mainLayout);

    qDebug() << "Form:" << this->objectName()
             << "sizeHint =" << this->sizeHint()
             << "minSizeHint =" << this->minimumSizeHint();



}

TxVoiceRigControlForm::~TxVoiceRigControlForm()
{

}



void TxVoiceRigControlForm::setKeyerAvailableIndicatorOnOff(bool on)
{
    indicators.keyerIndicators->setKeyerAvailableIndicatorOnOff(on);
}

void TxVoiceRigControlForm::setRepeatIndicatorOnOff(bool on)
{
    indicators.keyerIndicators->setRepeatIndicatorOnOff(on);
}

void TxVoiceRigControlForm::setEOMLabelText(const QString text)
{
    indicators.keyerIndicators->setEOMLabelText(text);
}

void TxVoiceRigControlForm::clearEOMLabelText()
{
    indicators.keyerIndicators->clearEOMLabelText();
}

void TxVoiceRigControlForm::setPttEnabledIndicatorOnOff(const bool on)
{
    indicators.pttIndicator->setPttEnabledIndicator(on);
}


void TxVoiceRigControlForm::setTxStatusIndicatorOnOff(const bool on)
{
    indicators.pttIndicator->setTxStatusIndicator(on);
}

void TxVoiceRigControlForm::setPttTypeText(const QString text)
{
    indicators.pttIndicator->setPttTypeText(text);
}

void TxVoiceRigControlForm::clearPttTypeText()
{
    indicators.pttIndicator->clearPttTypeText();
}

/*
void TxVoiceRigControlForm::setErrorMessageDisplayText(const QString errormsg)
{
    keyerErrorMessageDisplay->setErrorMessage(errormsg);
}

void TxVoiceRigControlForm::clearErrorMessageDisplayText()
{
    keyerErrorMessageDisplay->clearErrorMessage();
}


void TxVoiceRigControlForm::showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour)
{
   keyerErrorMessageDisplay->showTemporaryErrorMessage(msg, timeoutMs, colour);
}

*/
