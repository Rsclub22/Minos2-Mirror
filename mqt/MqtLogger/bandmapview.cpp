#include "bandmapview.h"

BandmapView::BandmapView(QWidget *parent) : QWidget(parent)
{
    bandmap = qobject_cast<Bandmap*>(parent);
    Q_ASSERT(bandmap);
    setFocusPolicy((Qt::WheelFocus));
    setMinimumSize(minimumSizeHint());
    dial = new BandmapFreqDial();


}


QSize BandmapView::minimumSizeHint() const
{

}


QSize BandmapView::sizeHint() const
{

}


bool BandmapView::eventFilter(QObject *target, QEvent *event)
{

}


void BandmapView::mousePressEvent(QMouseEvent *event)
{

}


void BandmapView::keyPressEvent(QKeyEvent *event)
{

}


void BandmapView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    dial->drawScale(&painter, curFreq, 800);
    dial->drawCursor(&painter, curFreq);
}


void BandmapView::setFreq(double f)
{

        curFreq = f;
        update();


}
