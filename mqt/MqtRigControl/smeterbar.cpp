#include "smeterbar.h"



#include <QPainter>
#include <QDebug>




SmeterBar::SmeterBar(QWidget *parent)
    :   QWidget(parent)
    ,   slevel(0.0)
    ,   levelColor(Qt::green)

{

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(15);


}

SmeterBar::~SmeterBar()
{

}

void SmeterBar::reset()
{
    slevel = 0;
    update();
}

void SmeterBar::setLevel(qreal _level)
{

    slevel = _level;
    if (slevel < 0.0)
    {
        slevel = 0.0;
    }
    else if (slevel > 1.0)
    {
        slevel = 1.0;
    }

    update();

}



void SmeterBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), Qt::darkGray);

    if (slevel != 0.0)
    {
        QRect bar = rect();


        bar.setRight(static_cast<int>(slevel * rect().right()));
        painter.fillRect(bar, levelColor);

    }



}
