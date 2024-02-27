#include "minoscustomtab.h"



#include <QPainter>

MinosCustomTab::MinosCustomTab(QWidget *parent) : QWidget(parent)
{

}

void MinosCustomTab::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), QColor(colour)); // Set your desired background color here
}


void MinosCustomTab::setBackgroundColour(QString colour_)
{
    colour = colour_;
    update();
}
