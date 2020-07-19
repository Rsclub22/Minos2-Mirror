#ifndef BANDMAPFREQDIAL_H
#define BANDMAPFREQDIAL_H
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>
#include "base_pch.h"
#include "bandmapcommon.h"
#include "checkoperatingfreq.h"



class DialFreqText
{

public:

    DialFreqText(QRect _textRect, qint32 _freqText)
    {
        textRect = _textRect;
        freqText = _freqText;
    }

    QRect getTextRect(){return textRect;}
    qint32 getFreqText(){return freqText;}



private:

    QRect textRect;
    qint32 freqText;

};




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

    qint64 getScaleStartFreq();
    qint64 getScaleEndFreq();


    void setCurWidth(int width);
    int getCurWidth();

    void changeBoundingRect(int height, int width);
    int checkFreqWidth(qint32 freq);
    int checkFreqWidth(double freq);


    void onFontChanged(QFont cf);
    void setCursorColour(QColor colour);

    int getYCoordOnDial(qint64 frequency);


//    void drawDial(QPainter *painter);

//    void calcStartEndFreq(qint32 frequency);
    qint32 getCurFreqInt32();
    QString getFreqFromYCoordOnDial(int y);
    qint32 checkSelectedFreqTextOnDial(QPoint p);


    int getFullBandHeight(double flow, double fhigh);


    qint64 getViewPortFreq(int startPos, double contestBandFlow);

    void setViewPortStartEndFreq(int startPos, int endPos, double contestBandFlow);


    void setContestBandLimits(double flow, double fhigh);
    void setFreqOperatingInfo(const QString contestBandStr, const QString contestModeStr, CheckOperatingFreq *operatingFreq, const bool operatingPlanOk);

    void setRadioMode(QString mode);
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
    qint64 scaleStartFreq = 0;
    qint64 scaleEndFreq = 0;
    qint32 currentFreqInt32 = 0;
    qint64 currentFreqInt64 = 0;

    int scaleStartYCoord;
    int scaleEndYCoord;
    int fullBandHeight;

    qint32 contestBandFlow;
    qint32 contestBandFhigh;

    QString contestBandStr;
    QString contestModeStr;
    CheckOperatingFreq *operatingFreq;
    bool operatingPlanOk;

    QString radioMode;



    QColor cursorColour;

    void changeZoom(bool direction);

//    QPainter  *painter;


    QString convertFreqDialDisplay(qint32 freq);

    int getFontHeight();

    QList< QSharedPointer<DialFreqText> > dialFreqList;
    int readBandmapZoomLevel();
    void saveBandmapZoomLevel(int &level);
};

#endif // BANDMAPFREQDIAL_H
