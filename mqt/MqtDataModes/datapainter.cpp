#include <QPainter>

#include "datapainter.h"


DataPainter::DataPainter(QWidget *parent)
    : QWidget{parent}
{

}
void DataPainter:: paintEvent(QPaintEvent *e)
{
    QPainter myPainter(this);

    myPainter.setFont(QFont("Times", 10, QFont::Normal));

    int yoffset = 40;
    for(const auto &s:qAsConst(text))
    {
        myPainter.drawText(QPoint(20, yoffset), s);
        yoffset += 40;
    }
}
