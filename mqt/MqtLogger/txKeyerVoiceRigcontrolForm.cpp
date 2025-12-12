#include <QDebug>
#include "txKeyerVoiceRigcontrolForm.h"


TxVoiceRigControlForm::TxVoiceRigControlForm(QWidget *parent)
    : QWidget(parent)
{


    indicatorLayout = new QHBoxLayout();
    indicatorLayout->setContentsMargins(2, 0, 0, 2);
    indicatorLayout->setSpacing(6);

    keyerIndicatorsWidget = new KeyerIndicatorsWidget();
    pttIndicatorWidget = new PttIndicatorWidget();

    keyerIndicatorsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    keyerIndicatorsWidget->setMaximumHeight(keyerIndicatorsWidget->sizeHint().height());

    pttIndicatorWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    pttIndicatorWidget->setMaximumHeight(keyerIndicatorsWidget->sizeHint().height());

    QSpacerItem *hSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);


    indicatorLayout->addWidget(keyerIndicatorsWidget, 1);   //stretch 1
    indicatorLayout->addWidget(pttIndicatorWidget, 1);
    indicatorLayout->addSpacerItem(hSpacer);

    errorMessageLayout = new QHBoxLayout();
    keyerErrorMessageWidget = new KeyerErrorMessageWidget();
    keyerErrorMessageWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    keyerErrorMessageWidget->setMaximumHeight(keyerErrorMessageWidget->sizeHint().height());
    errorMessageLayout->addWidget(keyerErrorMessageWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);
    mainLayout->addLayout(indicatorLayout);
    mainLayout->addLayout(errorMessageLayout);

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
    keyerIndicatorsWidget->setKeyerAvailableIndicatorOnOff(on);
}

void TxVoiceRigControlForm::setRepeatIndicatorOnOff(bool on)
{
    keyerIndicatorsWidget->setRepeatIndicatorOnOff(on);
}

void TxVoiceRigControlForm::setEOMLabelText(const QString text)
{
    keyerIndicatorsWidget->setEOMLabelText(text);
}

void TxVoiceRigControlForm::clearEOMLabelText()
{
    keyerIndicatorsWidget->clearEOMLabelText();
}

void TxVoiceRigControlForm::setPttEnabledIndicatorOnOff(const bool on)
{
    pttIndicatorWidget->setPttEnabledIndicator(on);
}


void TxVoiceRigControlForm::setTxStatusIndicatorOnOff(const bool on)
{
    pttIndicatorWidget->setTxStatusIndicator(on);
}

void TxVoiceRigControlForm::setPttTypeText(const QString text)
{
    pttIndicatorWidget->setPttTypeText(text);
}

void TxVoiceRigControlForm::clearPttTypeText()
{
    pttIndicatorWidget->clearPttTypeText();
}


void TxVoiceRigControlForm::setErrorMessageDisplayText(const QString errormsg)
{
    keyerErrorMessageWidget->setErrorMessage(errormsg);
}

void TxVoiceRigControlForm::clearErrorMessageDisplayText()
{
    keyerErrorMessageWidget->clearErrorMessage();
}


void TxVoiceRigControlForm::showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour)
{
    keyerErrorMessageWidget->showTemporaryErrorMessage(msg, timeoutMs, colour);
}

