#include "pttindicatorwidget.h"
#include "clustercommon.h"

PttIndicatorWidget::PttIndicatorWidget(QWidget *parent)
    :QFrame(parent)
{

    pttGrpBox = new QGroupBox("Ptt", this);

    layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);


    QLabel *enabledLbl = new QLabel();
    enabledLbl->setText("Enabled");
    pttEnabledIndicator = new QPushButton();
    pttEnabledIndicator->setMaximumSize(16, 16);
    pttEnabledIndicator->setIconSize(QSize(16, 16));

    QLabel *txStatusLbl = new QLabel();
    txStatusLbl->setText("TxStatus");
    txStatusIndicator = new QPushButton();
    txStatusIndicator->setMaximumSize(16, 16);
    txStatusIndicator->setIconSize(QSize(16, 16));

    QLabel *typeLbl = new QLabel();
    typeLbl->setText("Type: ");
    pttTypeText = new QLabel();

    QFrame *vLine = new QFrame(this);
    vLine->setFrameShape(QFrame::VLine);
    vLine->setFrameShadow(QFrame::Sunken);


    layout->addWidget(enabledLbl);
    layout->addWidget(pttEnabledIndicator);

    layout->addWidget(vLine);

    layout->addWidget(txStatusLbl);
    layout->addWidget(txStatusIndicator);

    layout->addWidget(vLine);
    layout->addWidget(typeLbl);
    layout->addWidget(pttTypeText);

    pttGrpBox->setLayout(layout);

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
