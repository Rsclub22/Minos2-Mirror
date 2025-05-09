/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QtWidgets>
#include <QStringList>
#include <QFont>
#include <QtMath>

#include "minoscompass.h"

/*
// for test
#include <QTimer>
#include <QString>
// for test
*/


MinosCompass::MinosCompass(QWidget *parent)

    : QWidget(parent)

{
    compassDialBearing = 0;
    setSizePolicy(QSizePolicy:: Preferred, QSizePolicy:: Preferred);

    setMouseTracking(true);

    installEventFilter(this);
   // for test...
//   QTimer *timer = new QTimer(this);
//   connect(this, SIGNAL(bearing_updated(QString)), this, SLOT(compassDialUpdate(const QString &)));
//   connect(timer, SIGNAL(timeout()), this, SLOT(upDateDial()));
//    timer->start();
   // for test
}

QSize MinosCompass::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}
QSize MinosCompass::sizeHint() const
{
    return QSize(100, 100);
}
/*
// for test
void MinosCompass::upDateDial()
{

    static int bearing = 0;

    bearing += 1;
    if (bearing > 359)
    {
        bearing = 0;
    }
    QString s = QString::number(bearing);
    emit bearing_updated(s);
}
// for test
*/
void MinosCompass::paintEvent(QPaintEvent *)
{
    static const QPoint needleFront[3] = {
        QPoint(7, 0),
        QPoint(-7, 0),
        QPoint(0, -80)
    };

    static const QPoint needleBack[3] = {
        QPoint(7, 0),
        QPoint(-7, 0),
        QPoint(0, 80)
    };


    QColor tenDegreeMarkerColor(127, 0, 127);
    QColor twoDegreeMarkerColor(0, 127, 127, 191);

    QColor needleFrontColor("red");
    QColor needleBackColor("gray");
    QColor mouseBearingColor("green");
    QColor annulusColor("aliceblue");
    annulusColor = annulusColor.darker(110);

    int side = qMin(width(), height());

    QPainter painter(this);

    painter.translate(width() / 2, height() / 2);

    painter.scale(side / 200.0, side / 200.0);

    QPainterPath qpp;
    qpp.addEllipse(QRect(-100, -100, 200, 200));
    qpp.addEllipse(QRect(-70, -70, 140, 140));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(annulusColor));

    painter.drawPath(qpp);

    // Draw the skyScan annulus segment
    if (skyScanStartBearing != skyScanEndBearing)
    {
        drawSkyScanAnnulusSegment(&painter, skyScanStartBearing, skyScanEndBearing);
    }

    QFont dialfont = dynamic_cast<QWidget *>(parent())->font();
    qreal dps = dialfont.pointSizeF();
    dialfont.setPointSizeF(dps * 250.0 / side );
    painter.setFont(dialfont);

    painter.setPen(Qt::NoPen);
    painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );


    // paint ten degree markers
    painter.setPen(tenDegreeMarkerColor);


    for (int i = 0; i < 36; ++i) {
       painter.drawLine(88, 0, 96, 0);
       painter.rotate(10.0);
    }

    if (mouseBearing >= 0)
    {
        painter.setPen(mouseBearingColor);
        painter.rotate(mouseBearing);
        painter.drawLine(00, -100, 0, -70);//x1, y1, x2, y2
        painter.rotate(-mouseBearing);
    }

    painter.setPen(Qt::NoPen);


    painter.setBrush(needleFrontColor);

    painter.save();

    painter.rotate(compassDialBearing);
    painter.drawConvexPolygon(needleFront, 3);

    painter.setBrush(needleBackColor);
    painter.drawConvexPolygon(needleBack, 3);

    painter.restore();


    painter.setPen(twoDegreeMarkerColor);


    for (int j = 0; j < 180; ++j) {
        if ((j % 5) != 0){
            painter.drawLine(92, 0, 96, 0);
       }
       painter.rotate(2);
    }

     painter.setPen("black");



    QString degLegends = " ,30,60, ,120,150, ,210,240, ,300,330";
    int degreeRotate[] = {30,30,30,30,30,30,30,30,30,30,30,30};
    QStringList legendsList = degLegends.split(",");

    QRect trect = painter.fontMetrics().boundingRect("XXX");
    trect.adjust(-trect.width()/2, -75, -trect.width()/2, -75);


    for (int j = 0; j < 12; j++)
    {
        painter.drawText(trect, Qt::AlignCenter,legendsList.value(j));
        painter.rotate(degreeRotate[j]);
    }

    QString dirLegends = tr("N,E,S,W");
    QStringList dirLegendsList = dirLegends.split(",");

    QRect tr2 = painter.fontMetrics().boundingRect("XXX");
    tr2.adjust(-trect.width()/2, -75, -trect.width()/2, -75);

    for (int j = 0; j < 4; j++)
    {
        painter.drawText(tr2, Qt::AlignCenter,dirLegendsList.value(j));
        painter.rotate(90);
    }


}

double angleFromN(const QPoint &V )
{
    return atan2(-V.y(), V.x());    // mouse position increases down and right
}


int MinosCompass::getMouseBearing(QPoint vec)
{
    double brg = angleFromN(vec) ;
    brg *= -180/M_PI;    // clockwise degrees
    brg += 90;      // from N rather than E

    while (brg < 0)
        brg += 360;
    while (brg > 360)
        brg -= 360;
    int m = static_cast<int>(brg);

    return m;
}
bool MinosCompass::inAnnulus(QPoint vec)
{
    double distanceFromCentre = sqrt(vec.x() * vec.x() + vec.y() * vec.y());

    int minwh = std::min(width(), height());

    int radius = minwh/2;
    int inner = (radius * 7)/10;

    bool ina = distanceFromCentre < radius && distanceFromCentre > inner;

    return ina;
}
void MinosCompass::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPoint lastPoint = event->pos();
        QPoint centre(width()/2, height()/2);

        QPoint vec = lastPoint - centre;

        if (inAnnulus(vec))
        {
            int brg = getMouseBearing(vec) ;
            emit sendClickBearing(static_cast<int>(brg) );
        }
        else
        {
            emit sendStop();
        }
    }
}
void MinosCompass::mouseMoveEvent(QMouseEvent *event)
{
    QPoint lastPoint = event->pos();
    QPoint centre(width()/2, height()/2);

    QPoint vec = lastPoint - centre;

    if (inAnnulus(vec))
    {
        mouseBearing = getMouseBearing(vec);

        update();
    }
    else
    {
        if (mouseBearing != -1)
        {
            mouseBearing = -1;
            update();
        }
    }
    QToolTip::hideText();
}

bool MinosCompass::eventFilter(QObject */*obj*/, QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = dynamic_cast<QHelpEvent *>(event);

        if (helpEvent)
        {
            QString message;

            QPoint lastPoint = helpEvent->pos();
            QPoint centre(width()/2, height()/2);

            QPoint vec = lastPoint - centre;
            if (inAnnulus(vec))
            {
                message = QString("%1").arg(getMouseBearing(vec));
            }
            else
            {
                message = tr("Stop");
            }

            if (!QToolTip::isVisible())
            {
                QPoint p = mapFromGlobal(QCursor::pos());
                if (p.x() >= 0 && p.y() >= 0 && p.x() < width() && p.y() < height())
                {
                    QToolTip::showText(QCursor::pos(), message);
                }
            }
        }
        return true;
    }
    return QWidget::event(event);
}

void MinosCompass::drawSkyScanAnnulusSegment(QPainter *painter, int rotatorStartBearing, int rotatorEndBearing)
{
    if (rotatorStartBearing == rotatorEndBearing)
        return; // Nothing to draw

    QList<SkyScanArcSegment> segments = splitBearingArc(rotatorStartBearing, rotatorEndBearing, endStopType);

    for (const SkyScanArcSegment &seg : segments)
    {
        int adjustedStart = mod360(seg.startAngle + antennaOffset -90);
        int adjustedEnd = mod360(seg.endAngle + antennaOffset - 90);
        drawAnnulusArc(painter, seg.isInnerArc, adjustedStart, adjustedEnd, true, seg.colour);
    }
}




void MinosCompass::drawAnnulusArc(QPainter *painter, bool innerArc,
                                  int startAngle, int endAngle, bool clockwise, const QColor &pathColor)
{

    QRectF innerRect;
    QRectF outerRect;


    if (innerArc)
    {
        innerRect = overlapInnerRect;
        outerRect = overlapOuterRect;
    }
    else
    {
        innerRect = mainInnerRect;
        outerRect = mainOuterRect;
    }


    double start = static_cast<double>(startAngle);
    double end = static_cast<double>(endAngle);

    double sweep = end - start;
    if (sweep <= 0)
        sweep += 360;

    if (!clockwise)
        sweep = -sweep;

    QPainterPath arcPath;
    arcPath.arcTo(outerRect, -start, -sweep);
    arcPath.arcTo(innerRect, -end, sweep);
    arcPath.closeSubpath();

    painter->setBrush(pathColor);
    painter->setPen(Qt::NoPen);
    painter->drawPath(arcPath);
}



QList<SkyScanArcSegment> MinosCompass::splitBearingArc(int rotatorStart, int rotatorEnd, endStop type)
{
    QList<SkyScanArcSegment> segments;

    bool ascending = false;

    auto addSegment = [&](int s, int e, const QColor &color, bool inner) {
        segments.append(SkyScanArcSegment{s, e, color, inner});
    };

    if (rotatorStart < rotatorEnd)
    {
        ascending = true;   // we have already set to false for descending
    }






    if (type == ROT_0_450)
    {
        if (ascending)
        {

            // rotatorStart < rotatorEnd
            if (rotatorStart < 360 && rotatorEnd > 360)
            {
                addSegment(rotatorStart, 360, mainColor, false);
                addSegment(360, rotatorEnd, overlapColor, true);
            }
            else if (rotatorStart >= 360)
            {
                addSegment(rotatorStart, rotatorEnd, overlapColor, true);
            }
            else
            {
                addSegment(rotatorStart, rotatorEnd, mainColor, false);
            }
        }
        else
        {
            // rotatorStart > rotatorEnd

            if (rotatorEnd < 360 && rotatorStart > 360)
            {
                addSegment(360, rotatorStart, overlapColor, true);
                addSegment(rotatorEnd, 360, mainColor, false);
            }
            else if (rotatorStart >= 360 && rotatorEnd >= 360)
            {
                addSegment(rotatorEnd, rotatorStart, overlapColor, true);
            }
            else
            {
                addSegment(rotatorEnd, rotatorStart, mainColor, false);
            }
        }

    }
    else if (type == ROT_NEG180_450)
    {

    }
    else if (type == ROT_NEG180_540)
    {

        if (ascending)
        {
            // rotatorStart < rotatorEnd
            if (rotatorStart < 0 && rotatorEnd <= 0)
            {
                //addSegment(rotatorStart + 360, rotatorEnd + 360, overlapColor, true);
            }
            else if (rotatorStart < 0 && rotatorEnd > 0)
            {
                //addSegment(rotatorStart + 360, 360, overlapColor, true);
                //addSegment(0, rotatorEnd, mainColor, false);
            }
            else if (rotatorStart < 360 && rotatorEnd > 360)
            {
                //addSegment(rotatorStart, 360, mainColor, false);
                //addSegment(360, rotatorEnd, negativeOverlapColor, true);
            }
            else if (rotatorStart >= 360)
            {
                //addSegment(rotatorStart, rotatorEnd, negativeOverlapColor, true);
            }
            else
            {
                //addSegment(rotatorStart, rotatorEnd, mainColor, false);
            }
        }
        else
        {

        }
    }
    else if (type == ROT_NEG179_180)
    {

    }
    else if (type == ROT_0_360)
    {
        if (ascending)
        {
            addSegment(rotatorStart, rotatorEnd, mainColor, false);
        }
        else
        {
            addSegment(rotatorEnd, rotatorStart, mainColor, false);
        }
    }





    return segments;
}







int MinosCompass::mod360(int bearing)
{
    while (bearing < 0) bearing += 360;
    return bearing % 360;
}



void MinosCompass::updateEndStopType(int endStopType_)
{
    endStopType = static_cast<endStop>(endStopType_);
}

void MinosCompass::updateSouthStopType(int southStopType_)
{
    southStopType = static_cast<southStop>(southStopType_);
}

void MinosCompass::updateAntennaOffset(int ant_offset)
{
    antennaOffset = ant_offset;
}

void MinosCompass::updateSkyScanStartBearing(int bearing)
{
    if (southStopType == S_STOP_COMPASS_SENSOR)
    {
        if (bearing < 0)
        {
            bearing = bearing + COMPASS_MAX360;
        }
    }

    skyScanStartBearing = bearing;
    qDebug() << "skyScan compass start bearing = " << QString::number(skyScanStartBearing);
    update();
}

void MinosCompass::updateSkyScanEndBearing(int bearing)
{
    if (southStopType == S_STOP_COMPASS_SENSOR)
    {
        if (bearing < 0)
        {
            bearing = bearing + COMPASS_MAX360;
        }
    }

    skyScanEndBearing = bearing;
    qDebug() << "skyScan compass end bearing = " << QString::number(skyScanEndBearing);
    update();
}

void MinosCompass::compassDialUpdate(int bearing)
{

    compassDialBearing = bearing;
    update();

}

