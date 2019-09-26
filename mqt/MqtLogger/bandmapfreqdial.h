#ifndef BANDMAPFREQDIAL_H
#define BANDMAPFREQDIAL_H
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>
#include "base_pch.h"
#include "bandmapcommon.h"

namespace dialData {



const int khzStep[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 25, 50, 50, 50};
const int khzPixelStep[] = {200, 150, 110, 85, 65, 50, 35, 25, 20, 15, 11, 8, 6, 4, 3, 2, 1};
const int hzPixelStep[] = {5, 6, 9, 12, 15, 20, 28, 40, 50, 66, 90, 125, 166, 250, 333, 500, 1000};
const int minorMarker[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 3, 0, 0, 0};
const int roundFactor[] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 10, 10, 10};

const int fMajMrkXStart = 50;
const int fMajMrkXEnd = 70;
const int fMajMrkLength = 20;
const int fMajTextXStart = 5;

const int fMinMrkXStart = 60;
const int fMinMrkXEnd = 70;
const int fMinMrkLength = 10;

const int additionalWidth = 30;  // width in addition to freq text

const int MIN_ZOOM_LEVEL = 0;
const int MAX_ZOOM_LEVEL = 16;

const int MAXSCALEY = 675;
const int MAXSCALEX = 100;

const int DIAL_VERT_OFFSET = 10;      // dial offset to show first text

}






class BandmapFreqDial : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)


public:
    //BandmapFreqDial(int width, int height, QGraphicsItem *parent = nullptr);
    BandmapFreqDial(int width, int height);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QRectF boundingRect() const override;

    void setCurFreq(double frequency);
    double getCurFreq();
    void setCurHeight(int height);
    int getCurHeight();

    void drawScale(QPainter *painter, qint32 frequency, int scaleHeight);
    void drawCursor(QPainter *painter, qint64 frequency);


    void setZoomLevel(int level);
    int getZoomLevel();
    int getMaxZoomLevel() {return dialData::MAX_ZOOM_LEVEL;}
    int getMinZoomLevel() {return dialData::MIN_ZOOM_LEVEL;}

    qint32 getScaleStartFreq();
    qint32 getScaleEndFreq();


    void setCurWidth(int width);
    int getCurWidth();

    void changeBoundingRect(int height, int width);
    int checkFreqWidth(qint32 freq);
    int checkFreqWidth(double freq);


    void onFontChanged(QFont cf);
    void setCursorColour(QColor colour);

    int getYCoordOnDial(qint64 frequency);


//    void drawDial(QPainter *painter);

    void calcStartEndFreq(qint32 frequency);
    qint32 getCurFreqInt32();
    QString getFreqFromYCoordOnDial(int y);
signals:
    //void dialupdated();
    void zoomUpdated(bool);

protected:
    //void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void wheelEvent(QGraphicsSceneWheelEvent * event) override;


private:

    int zoomLevel = 0;
    int dialHeight = dialData::MAXSCALEY;
    int dialWidth  = dialData::MAXSCALEX;
    int newFreqTextWidth = 0;
    int freqTextWidth = 0;
    int fontHeight = 0;

    double currentFreqDbl;

    //qint32 currentFreq = 0;
    qint32 scaleStartFreq = 0;
    qint32 scaleEndFreq = 0;
    qint32 currentFreqInt32 = 0;
    qint64 currentFreqInt64 = 0;

    QColor cursorColour;

    void changeZoom(bool direction);

//    QPainter  *painter;


    QString convertFreqDialDisplay(qint32 freq);

    int getFontHeight();
};

#endif // BANDMAPFREQDIAL_H
