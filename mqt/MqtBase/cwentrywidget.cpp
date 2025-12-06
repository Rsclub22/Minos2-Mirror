#include "cwentrywidget.h"
#include "clustercommon.h"
#include "cwrigkeyervalidator.h"

#include <QLabel>



CwEntryWidget::CwEntryWidget(QWidget *parent)
    :QFrame(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    addCwEntryWidgets();
}


void CwEntryWidget::addCwEntryWidgets()
{
    QLabel *lbl = new QLabel();
    lbl->setText("CW");

    cwPlayingIndicator = new QPushButton();
    cwPlayingIndicator->setMaximumSize(16, 16);
    cwPlayingIndicator->setIconSize(QSize(16, 16));

    cwEntry = new QLineEdit();
    connect(cwEntry, &QLineEdit::returnPressed, this, [this ]() {
        emit cwEntryReturnPressed();
    });

    layout->addWidget(lbl);
    layout->addWidget(cwPlayingIndicator);
    layout->addWidget(cwEntry);



}


QString CwEntryWidget::getCwEntryText()
{
    return cwEntry->text();
}


void CwEntryWidget::setCwFreeTextIndicatorOnOff(bool on)
{
    if (on)
    {
        cwPlayingIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        cwPlayingIndicator->setToolTip(tr("CW FreeText Playing"));
    }
    else
    {
        cwPlayingIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
        cwPlayingIndicator->setToolTip(tr("No CW FreeText Playing"));
    }
}

void CwEntryWidget::setValidator(const QString validChars, const int maxNumChars)
{
    auto *validator = new CWRigKeyerValidator(this);
    validator->setValidCwCharStr(validChars);
    validator->setMaxNumCwChars(maxNumChars);
    cwEntry->setValidator(validator);
}
