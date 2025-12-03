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

struct SkyScanArcSegment {
    int startAngle;    // in degrees, raw (can be < 0 or > 360)
    int endAngle;
    bool isInnerArc;   // true = main annulus, false = outer annulus (red or green)
    QColor colour;
};



class MinosCompass : public QWidget
{
    Q_OBJECT

public:
    MinosCompass(QWidget *parent = nullptr);



    void setDoNotShowNeedle(bool doNotShowNeedle_);

    void setMouseEventsEnabled(bool state);

public slots:

    void updateSkyScanStartBearing(int bearing);
    void updateSkyScanEndBearing(int bearing);
    void compassDialUpdate(int);
    void updateEndStopType(int endStopType_);
    void updateSouthStopType(int southStopType_);
    void updateAntennaOffset(int ant_offset);
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
    bool event(QEvent *event) override;

private:

    int compassDialBearing;
    int mouseBearing = -1;
    int antennaOffset = 0;

    int skyScanStartBearing = 0;
    int skyScanEndBearing = 0;

    endStop endStopType = ROT_0_360;
    southStop southStopType = southStop::S_STOPOFF;

    bool doNotShowNeedle = false;
    bool disableMouseEvents = false;



    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    int getMouseBearing(QPoint vec);
    bool inAnnulus(QPoint vec);


    static constexpr qreal mainInnerRadius = 65;
    static constexpr qreal mainOuterRadius = 70;
    static constexpr qreal overlapInnerRadius = 60;
    static constexpr qreal overlapOuterRadius = 65;

    const QRectF mainInnerRect  = QRectF(-mainInnerRadius, -mainInnerRadius, mainInnerRadius * 2, mainInnerRadius * 2);
    const QRectF mainOuterRect  = QRectF(-mainOuterRadius, -mainOuterRadius, mainOuterRadius * 2, mainOuterRadius * 2);
    const QRectF overlapInnerRect = QRectF(-overlapInnerRadius, -overlapInnerRadius, overlapInnerRadius * 2, overlapInnerRadius * 2);
    const QRectF overlapOuterRect = QRectF(-overlapOuterRadius, -overlapOuterRadius, overlapOuterRadius * 2, overlapOuterRadius * 2);

    const QColor mainColor = QColor("#8ecaff");
    const QColor overlapColor = QColor("#ffa4aa");
    const QColor negativeOverlapColor = QColor("#e0c31e");

    void drawSkyScanAnnulusSegment(QPainter *painter, int rotatorStartBearing, int rotatorEndBearing);



    QList<SkyScanArcSegment> splitBearingArc(int rotatorStart, int rotatorEnd, endStop type);
    void drawAnnulusArc(QPainter *painter, bool innerArc, int startAngle, int endAngle, const QColor &pathColor);
    static int mod360(int bearing);



};


#endif
