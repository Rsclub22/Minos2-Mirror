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
#include "qpainterpath.h"
#include "rotatorcommon.h"


class skyScanAnnulusParameters
{
public:

    skyScanAnnulusParameters(){}


    QPainter* painter;

    QPainterPath mainPath;
    QPainterPath overlapPath;

    double mainInnerRadius = 65;
    double mainOuterRadius = 70;
    double overLapInnerRadius = 60;
    double overLapOuterRadius = 65;

    QColor mainColor = "#8ecaff";
    QColor overlapColor = "#ffa4aa";
    QColor negativeOverlapColor = "#e0c31e";

    double rotatorStartBearing = 0;
    double rotatorEndBearing = 0;

    // Adjust for Qt coords
    double adjustedStartBearing = 0;
    double adjustedEndBearing = 0;

    // Define the outer arc's bounding rectangle
    QRectF mainOuterRect = QRectF(-mainOuterRadius, -mainOuterRadius, mainOuterRadius * 2, mainOuterRadius * 2);

    // Define the inner arc's bounding rectangle
    QRectF mainInnerRect = QRectF(-mainInnerRadius, -mainInnerRadius, mainInnerRadius * 2, mainInnerRadius * 2);

    QRectF overlapOuterRect = QRectF(-overLapOuterRadius, -overLapOuterRadius, overLapOuterRadius * 2, overLapOuterRadius * 2);

    // Define the inner arc's bounding rectangle
    QRectF overlapInnerRect = QRectF(-overLapInnerRadius, -overLapInnerRadius, overLapInnerRadius * 2, overLapInnerRadius * 2);


};

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
    void updateSouthStopType(int southStopType_);
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
    southStop southStopType = southStop::S_STOPOFF;

    using DrawSegmentFunction = std::function<void(
        QPainterPath&,
        QRectF&,
        QRectF&,
        double,
        double,
        double,
        QColor&
        )>;




    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    int getMouseBearing(QPoint vec);
    bool inAnnulus(QPoint vec);
    void drawSkyScanAnnulusSegment(QPainter *painter, int rotatorStartBearing, int rotatorEndBearing);
    void drawAnnulusArc(QPainter *painter, QPainterPath &arcPath, QRectF &innerRect, QRectF &outerRect, double startAngle, double endAngle, double sweepLength, QColor &pathColor);
    //void drawStandard_0_360_SkyScanPath(QPainter *painter, QColor &pathColor, QPainterPath arcPath, double rotatorStartBearing, double rotatorEndBearing, double adjustedStartBearing, double adjustedEndBearing, QRectF &innerRect, QRectF &outerRect);
    //void handle_0_540_RotatorsSkyScanPath(QPainter *painter, QColor &mainPathColor, QColor &overlapPathColor, QPainterPath mainArcPath, QPainterPath overlapPath, double rotatorStartBearing, double rotatorEndBearing, double adjustedStartBearing, double adjustedEndBearing, QRectF &mainInnerRect, QRectF &mainOuterRect, QRectF &overlapInnerRect, QRectF &overlapOuterRect);
    //void draw_0_540_ExtendedSkyScanPath(QPainter *painter, QColor &mainPathColor, QColor &overlapPathColor, QPainterPath mainPath, QPainterPath overlapPath, double rotatorStartBearing, double rotatorEndBearing, double adjustedStartBearing, double adjustedEndBearing, QRectF &mainInnerRect, QRectF &mainOuterRect, QRectF &overlapInnerRect, QRectF &overlapOuterRect);
    //void handle_NEG180_540_SkyScanPath(QPainter *painter, QColor &mainPathColor, QColor &overlapPathColor, QPainterPath mainPath, QPainterPath overlapPath, double rotatorStartBearing, double rotatorEndBearing, double adjustedStartBearing, double adjustedEndBearing, QRectF &mainInnerRect, QRectF &mainOuterRect, QRectF &overlapInnerRect, QRectF &overlapOuterRect);


    //void skyScanDrawAnnulusAscending(QPainter *painter, QColor &mainColor, QColor &overlapColor, QPainterPath &mainPath, QPainterPath &overlapPath, double rotatorStartBearing, double rotatorEndBearing, double adjustedStartBearing, double adjustedEndBearing, QRectF &mainInnerRect, QRectF &mainOuterRect, QRectF &overlapInnerRect, QRectF &overlapOuterRect);
    //void skyScanDrawAnnulusDescending(QPainter* painter, QColor &mainColor, QColor &overlapColor, QPainterPath &mainPath, QPainterPath &overlapPath, double rotatorStartBearing, double rotatorEndBearing, double adjustedStartBearing, double adjustedEndBearing, QRectF &mainInnerRect, QRectF &mainOuterRect, QRectF &overlapInnerRect, QRectF &overlapOuterRect );

    void skyScanDrawAnnulusAscending(skyScanAnnulusParameters &skyScanAnnulusParam);
    void skyScanDrawAnnulusDescending(skyScanAnnulusParameters &skyScanAnnulusParam);


};


#endif
