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

    DialFreqText(QRect _textRect, Frequency _freqText)
    {
        textRect = _textRect;
        freqText = _freqText;
    }

    QRect getTextRect(){return textRect;}
    Frequency getFreqText(){return freqText;}



private:

    QRect textRect;
    Frequency freqText;

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

    void setCurFreq(const Frequency &frequency);
    Frequency getCurFreq();
    void setCurHeight(int height);
    int getCurHeight();

    void drawScale(QPainter *painter, Frequency frequency, int scaleHeight);
    void drawCursor(QPainter *painter, Frequency frequency);


    void setZoomLevel(int level);
    int getZoomLevel();
    int getMaxZoomLevel() {return dialData::MAX_ZOOM_LEVEL;}
    int getMinZoomLevel() {return dialData::MIN_ZOOM_LEVEL;}

    Frequency getScaleStartFreq();
    Frequency getScaleEndFreq();


    void setCurWidth(int width);
    int getCurWidth();

    void changeBoundingRect(int height, int width);
    int checkFreqWidth(const Frequency &freq);


    void onFontChanged(QFont cf);
    void setCursorColour(QColor colour);

    int getYCoordOnDial(const Frequency &frequency);


    Frequency getFreqFromYCoordOnDial(int y);
    Frequency checkSelectedFreqTextOnDial(QPoint p);


    int getFullBandHeight(const Frequency &flow, const Frequency &fhigh);


    Frequency getViewPortFreq(int startPos, Frequency contestBandFlow);

    void setViewPortStartEndFreq(int startPos, int endPos, Frequency contestBandFlow);


    void setContestBandLimits(const Frequency &flow, const Frequency &fhigh);
    void setFreqOperatingInfo(const QString contestBandStr, const QString contestModeStr, CheckOperatingFreq *operatingFreq, const bool operatingPlanOk);

    void setRadioMode(QString mode);
signals:
    //void dialupdated();
    void zoomUpdated(bool);

protected:
    //void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void wheelEvent(QGraphicsSceneWheelEvent * event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;


private:

    int zoomLevel = 0;
    int dialHeight = dialData::MAXSCALEY;
    int dialWidth  = dialData::MAXSCALEX;
    int newFreqTextWidth = 0;
    int freqTextWidth = 0;
    int fontHeight = 0;

    Frequency currentFreq;

    Frequency scaleStartFreq;
    Frequency scaleEndFreq;

    int scaleStartYCoord;
    int scaleEndYCoord;
    int fullBandHeight;

    Frequency contestBandFlow;
    Frequency contestBandFhigh;

    QString contestBandStr;
    QString contestModeStr;
    CheckOperatingFreq *operatingFreq;
    bool operatingPlanOk;

    QString radioMode;



    QColor cursorColour;

    void changeZoom(bool direction);

//    QPainter  *painter;


    QString convertFreqDialDisplay(const Frequency &freq);

    int getFontHeight();

    QList< QSharedPointer<DialFreqText> > dialFreqList;
    int readBandmapZoomLevel();
    void saveBandmapZoomLevel(int &level);
};

#endif // BANDMAPFREQDIAL_H
