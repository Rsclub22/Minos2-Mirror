#include "pttindicatorwidget.h"
#include "clustercommon.h"

PttIndicatorWidget::PttIndicatorWidget(QWidget *parent)
    :QFrame(parent)
{

    pttGrpBox = new QGroupBox("Ptt", this);

    layout = new QHBoxLayout();
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(6);

    auto makeVLine = [this]() {
        QFrame *l = new QFrame(this);
        l->setFrameShape(QFrame::VLine);
        l->setFrameShadow(QFrame::Sunken);
        return l;
    };

    QLabel *enabledLbl = new QLabel("Enabled");
    pttEnabledIndicator = new QPushButton();
    pttEnabledIndicator->setMaximumSize(16, 16);
    pttEnabledIndicator->setIconSize(QSize(16, 16));
    pttEnabledIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    QLabel *txStatusLbl = new QLabel("TxStatus");
    txStatusIndicator = new QPushButton();
    txStatusIndicator->setMaximumSize(16, 16);
    txStatusIndicator->setIconSize(QSize(16, 16));
    txStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    QLabel *typeLbl = new QLabel("Type: ");
    pttTypeText = new QLabel();


    layout->addWidget(enabledLbl);
    layout->addWidget(pttEnabledIndicator);

    layout->addWidget(makeVLine());

    layout->addWidget(txStatusLbl);
    layout->addWidget(txStatusIndicator);

    layout->addWidget(makeVLine());

    layout->addWidget(typeLbl);
    layout->addWidget(pttTypeText);

    pttGrpBox->setLayout(layout);

    // this helps to apply a layout when promoted..
    auto *outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(pttGrpBox);

}

QSize PttIndicatorWidget::sizeHint() const
{
    return QSize(50, 50);
}

QSize PttIndicatorWidget::minimumSizeHint() const
{
    return QSize(20, 20);
}



void PttIndicatorWidget::setPttEnabledIndicator(const bool on)
{
    if (on)
    {
        pttEnabledIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);

    }
    else
    {
        pttEnabledIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }
}
void PttIndicatorWidget::setTxStatusIndicator(const bool on)
{
    if (on)
    {
        txStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);

    }
    else
    {
        txStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }
}
void PttIndicatorWidget::setPttTypeText(const QString text)
{
    pttTypeText->setText(text);
}
void PttIndicatorWidget::clearPttTypeText()
{
    pttTypeText->clear();
}
