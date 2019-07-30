#ifndef FREQDIAL_H
#define FREQDIAL_H
#include <QPainter>
#include <QGraphicsItem>


namespace dialData {



const int khzStep[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 25, 50, 50, 50};
const int khzPixelStep[] = {200, 150, 110, 85, 65, 50, 35, 25, 20, 15, 11, 8, 6, 4, 3, 2, 1};
const int hzPixelStep[] = {5, 6, 9, 12, 15, 20, 28, 40, 50, 66, 90, 125, 166, 250, 333, 500, 1000};
const int minorMarker[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 3, 0, 0, 0};


const int fMajMrkXStart = 50;
const int fMajMrkXEnd = 70;
const int fMajMrkLength = 20;

const int fMinMrkXStart = 60;
const int fMinMrkXEnd = 70;
const int fMinMrkLength = 10;

const int MAXSCALEY = 675;
//const int MAXSCALEY = 775;

}






class FreqDial : public QGraphicsItem
{
public:
    FreqDial();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;

    void setCurFreq(double frequency);
    double getCurFreq();
    void setCurHeight(int height);
    int getCurHeight();


    void changeBoundingRect(int height);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
private:

    int zoomLevel = 0;
    int maxScaleY = dialData::MAXSCALEY;
    double currentFreq = 0;

    double scaleStartFreq = 0;
    double scaleEndFreq = 0;

//    QPainter  *painter;
    void drawScale(QPainter *painter, double frequency, int scaleHeight);
    void drawCursor(QPainter *painter, double frequency);

};

#endif // FREQDIAL_H
