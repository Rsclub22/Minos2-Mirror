/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef MINOSCOMPASS_H
#define MINOSCOMPASS_H

#include <QWidget>
#include "rotatorcommon.h"

class MinosCompass : public QWidget
{
    Q_OBJECT

public:
    MinosCompass(QWidget *parent = nullptr);


public slots:

    void updateSkyScanStartBearing(int bearing);
    void updateSkyScanEndBearing(int bearing);
    void compassDialUpdate(int);
    void updateEndStopType(int endStopType_);
/*
// for test
    void upDateDial();

signals:
    void bearing_updated(const QString);

// for test
*/

signals:
    void sendClickBearing(int brg);
    void sendStop();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    int compassDialBearing;
    int mouseBearing = -1;

    int skyScanStartBearing = 0;
    int skyScanEndBearing = 0;

    endStop endStopType = ROT_0_360;

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    int getMouseBearing(QPoint vec);
    bool inAnnulus(QPoint vec);
    void drawSkyScanAnnulusSegment(QPainter *painter, int rotatorStartBearing, int rotatorEndBearing);
    void drawAnnulusArc(QPainter *painter, QPainterPath &arcPath, QRectF &innerRect, QRectF &outerRect, double startAngle, double endAngle, double sweepLength, QColor &pathColor);
};


#endif
