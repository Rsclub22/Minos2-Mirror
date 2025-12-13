#include "keyerindicatorswidget.h"
#include "clustercommon.h"

KeyerIndicatorsWidget::KeyerIndicatorsWidget(QWidget *parent)
    :QFrame(parent)
{
    indicatorGrpBox = new QGroupBox("Keyer", this);

    layout = new QHBoxLayout();
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(6);

    auto makeVLine = [this]() {
        QFrame *l = new QFrame(this);
        l->setFrameShape(QFrame::VLine);
        l->setFrameShadow(QFrame::Sunken);
        return l;
    };


    QLabel *availLbl = new QLabel("Avail");
    keyerAvailIndicator = new QPushButton();
    keyerAvailIndicator->setMaximumSize(16, 16);
    keyerAvailIndicator->setIconSize(QSize(16, 16));
    keyerAvailIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    QLabel *repeatLbl = new QLabel("Repeat");
    repeatIndicator = new QPushButton();
    repeatIndicator->setMaximumSize(16, 16);
    repeatIndicator->setIconSize(QSize(16, 16));
    repeatIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    QLabel *eomLbl = new QLabel("EOM: ");
    eomText = new QLabel();

    layout->addWidget(availLbl);
    layout->addWidget(keyerAvailIndicator);

    layout->addWidget(makeVLine());

    layout->addWidget(repeatLbl);
    layout->addWidget(repeatIndicator);

    layout->addWidget(makeVLine());

    layout->addWidget(eomLbl);
    layout->addWidget(eomText);


    indicatorGrpBox->setLayout(layout);


    auto *outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(indicatorGrpBox);
    outerLayout->addStretch();

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
