#include "keyerindicatorswidget.h"
#include "clustercommon.h"

KeyerIndicatorsWidget::KeyerIndicatorsWidget(QWidget *parent)
    :QFrame(parent)
{
    indicatorGrpBox = new QGroupBox("Keyer", this);

    layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);


    QLabel *availLbl = new QLabel();
    availLbl->setText("Avail");
    keyerAvailIndicator = new QPushButton();
    keyerAvailIndicator->setMaximumSize(16, 16);
    keyerAvailIndicator->setIconSize(QSize(16, 16));

    QLabel *repeatLbl = new QLabel();
    repeatLbl->setText("Repeat");
    repeatIndicator = new QPushButton();
    repeatIndicator->setMaximumSize(16, 16);
    repeatIndicator->setIconSize(QSize(16, 16));

    QLabel *eomLbl = new QLabel();
    eomLbl->setText("EOM: ");
    eomText = new QLabel();

    QFrame *vLine = new QFrame(this);
    vLine->setFrameShape(QFrame::VLine);
    vLine->setFrameShadow(QFrame::Sunken);


    layout->addWidget(availLbl);
    layout->addWidget(keyerAvailIndicator);

    layout->addWidget(vLine);

    layout->addWidget(repeatLbl);
    layout->addWidget(repeatIndicator);

    layout->addWidget(vLine);
    layout->addWidget(eomLbl);
    layout->addWidget(eomText);

    indicatorGrpBox->setLayout(layout);
}

void KeyerIndicatorsWidget::setKeyerAvailableIndicatorOnOff(bool on)
{
    if (on)
    {
        keyerAvailIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);

    }
    else
    {
        keyerAvailIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }
}




void KeyerIndicatorsWidget::setRepeatIndicatorOnOff(bool on)
{
    if (on)
    {
        repeatIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);

    }
    else
    {
        repeatIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }
}

void KeyerIndicatorsWidget::setEOMLabelText(const QString text)
{
    eomText->setText(text);
}

void KeyerIndicatorsWidget::clearEOMLabelText()
{
    eomText->clear();
}
