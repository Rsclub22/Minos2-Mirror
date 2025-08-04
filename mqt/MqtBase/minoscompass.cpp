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

    if (!doNotShowNeedle)
    {
        painter.setBrush(needleFrontColor);

        painter.save();

        painter.rotate(compassDialBearing);
        painter.drawConvexPolygon(needleFront, 3);

        painter.setBrush(needleBackColor);
        painter.drawConvexPolygon(needleBack, 3);

        painter.restore();

    }

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
bool MinosCompass::MinosCompass::event(QEvent *event)
{
    if (disableMouseEvents)
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::HoverMove:
        case QEvent::ToolTip:
            return true; // block
        default:
            break;
        }
    }

    return QWidget::event(event);
}

void MinosCompass::mousePressEvent(QMouseEvent *event)
{
    if (disableMouseEvents)
        return;

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
    if (disableMouseEvents)
        return;

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


void MinosCompass::setDoNotShowNeedle(bool doNotShowNeedle_)
{
    doNotShowNeedle = doNotShowNeedle_;
}

void MinosCompass::setMouseEventsEnabled(bool enabled)
{
    disableMouseEvents = !enabled;
}

bool MinosCompass::eventFilter(QObject */*obj*/, QEvent *event)
{

    if (event->type() == QEvent::ToolTip)
    {
        if (!disableMouseEvents)        // if disabled we don't want the tool tips
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

        }

        return true;
    }
    return QWidget::event(event);
}

void MinosCompass::drawSkyScanAnnulusSegment(QPainter *painter, int rotatorStartBearing, int rotatorEndBearing)
{
    if (rotatorStartBearing == rotatorEndBearing)
        return;

    QList<SkyScanArcSegment> segments = splitBearingArc(rotatorStartBearing, rotatorEndBearing, endStopType);

    for (const SkyScanArcSegment &seg : segments)
    {
        int adjustedStart = mod360(seg.startAngle + antennaOffset - 90);
        int adjustedEnd   = mod360(seg.endAngle   + antennaOffset - 90);

        drawAnnulusArc(painter, seg.isInnerArc, adjustedStart, adjustedEnd, seg.colour);
    }
}




void MinosCompass::drawAnnulusArc(QPainter *painter, bool innerArc,
                                  int startAngle, int endAngle, const QColor &pathColor)
{
    QRectF innerRect = innerArc ? overlapInnerRect : mainInnerRect;
    QRectF outerRect = innerArc ? overlapOuterRect : mainOuterRect;

    // Normalize both angles to 0–360 for drawing (visual only)
    double start = static_cast<double>(mod360(startAngle));
    double end   = static_cast<double>(mod360(endAngle));

    double sweep = end - start;
    if (sweep <= 0)
        sweep += 360;

    // The QPainter arc drawing is counter-clockwise, so negate angles
    QPainterPath arcPath;
    arcPath.arcTo(outerRect, -start, -sweep);
    arcPath.arcTo(innerRect, -end, sweep);
    arcPath.closeSubpath();

    painter->setBrush(pathColor);
    painter->setPen(Qt::NoPen);
    painter->drawPath(arcPath);
}

QList<SkyScanArcSegment> MinosCompass::splitBearingArc(int start, int end, endStop model)
{
    QList<SkyScanArcSegment> segments;

    if (start == end)
        return segments; // Nothing to draw

    // If start is greater than end, swap them to ensure clockwise drawing
    if (start > end)
    {
        int temp = start;
        start = end;
        end = temp;
    }

    auto addSegment = [&](int s, int e, bool inner, QColor c) {
        if (s != e) {
            segments.append({ s, e, inner, c });
        }
    };

    // Handle ROT_0_360
    if (model == ROT_0_360)
    {
        addSegment(start, end, true, mainColor);
    }
    // Handle ROT_0_450
    else if (model == ROT_0_450)
    {
        if (start < 360 && end <= 360)
        {
          addSegment(start, end, false, mainColor);
        }
        if (start < 360 && end > 360)
        {
            addSegment(0, 360, false, mainColor);
            addSegment(360, end, true, overlapColor);
        }
        if (start > 360 && end > 360)
        {
            addSegment(start, end, true, overlapColor);
        }
    }
    // Handle ROT_NEG179_180
    else if (model == ROT_NEG179_180)
    {
        if (start < 0 && end <= 0)
        {
           addSegment(start, end, true, negativeOverlapColor);
        }
        else if (start < 0 && end <= 180)
        {
            addSegment(start, 0, true, negativeOverlapColor);
            addSegment(0, end, false, mainColor);
        }
        else if (start >= 0 && end <= 180)
        {
            addSegment(start, end, false, mainColor);
        }


    }
    // Handle ROT_NEG180_540
    else if (model == ROT_NEG180_540)
    {
        if (start < 0 && end <= 0)
        {
            addSegment(start, end, true, negativeOverlapColor);
        }
        else if (start < 0 && end <= 360)
        {
            addSegment(start, 0, true, negativeOverlapColor);
            addSegment(0, end, false, mainColor);
        }
        else if (start >= 0 && end > 360)
        {
            addSegment(start, 360, false, mainColor);
            addSegment(360, end, true, overlapColor);
        }
        else if (start >= 0 && end <= 360)
        {
            addSegment(start, end, false, mainColor);
        }
        else if (start < 0 && end > 360)
        {
            addSegment(start, 0, true, negativeOverlapColor);
            addSegment(0, 360, false, mainColor);
            addSegment(360, end, true, overlapColor);
        }
        else if (start > 360 && end <= 540)
        {
            addSegment(start, end, true, overlapColor);
        }
    }
    else
    {
        // Default case, treat the entire range as main arc
        addSegment(start, end, true, mainColor);
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
    //qDebug() << "skyScan compass start bearing = " << QString::number(skyScanStartBearing);
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
    //qDebug() << "skyScan compass end bearing = " << QString::number(skyScanEndBearing);
    update();
}

void MinosCompass::compassDialUpdate(int bearing)
{

    compassDialBearing = bearing;
    update();

}

