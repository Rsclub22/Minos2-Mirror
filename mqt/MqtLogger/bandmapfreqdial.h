#ifndef BANDMAPFREQDIAL_H
#define BANDMAPFREQDIAL_H
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsSceneWheelEvent>
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
    Q_INTERFACES(QGraphicsItem) // This removes a warning; not sure it is useful!

public:
    BandmapFreqDial(int width, int height);
    void setHeight(int h);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QRectF boundingRect() const override;   // pure virtual in QGraphicsItem

    void setCurFreq(const Frequency &frequency);
    Frequency getCurFreq();
    int getCurHeight();

    void drawScale(QPainter *painter);
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
    int calcFreqWidth(const Frequency &freq);


    void onFontChanged(QFont cf);
    void setCursorColour(QColor colour);

    double getYCoordOnDial(const Frequency &frequency) const;


    Frequency getFreqFromYCoordOnDial(int y);
    Frequency checkSelectedFreqTextOnDial(QPoint p);


    int getFullBandHeight(const Frequency &flow, const Frequency &fhigh) const;

    void setViewPortStartEndFreq(int startPos, int endPos);

    void setContestBandLimits(const Frequency &flow, const Frequency &fhigh);
    void setFreqOperatingInfo(const QString contestBandStr, const QString contestModeStr, CheckOperatingFreq *operatingFreq, const bool operatingPlanOk);

    void setRadioMode(QString mode);

signals:

    void zoomUpdated(bool);

protected:
    //void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void wheelEvent(QGraphicsSceneWheelEvent * event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;


private:
    int height = 0;
    int zoomLevel = 0;
    int dialWidth  = dialData::MAXSCALEX;
    int newFreqTextWidth = 0;
    int freqTextWidth = 0;

    Frequency currentFreq;

    Frequency scaleStartFreq;
    Frequency scaleEndFreq;

    int scaleStartYCoord;
    int scaleEndYCoord;

    Frequency contestBandFlow;
    Frequency contestBandFhigh;

    QString contestBandStr;
    QString contestModeStr;
    CheckOperatingFreq *operatingFreq;
    bool operatingPlanOk;

    QString radioMode;

    QColor cursorColour;
    QFont panelFont;

    void changeZoom(bool direction);

    double getHzPixelStepR() const;

    QString convertFreqDialDisplay(const Frequency &freq);

    int getFontHeight() const;

    QList< QSharedPointer<DialFreqText> > dialFreqList;
    int readBandmapZoomLevel();
    void saveBandmapZoomLevel(int &level);

    void drawMarkerText(QPainter *painter, int ycoord, Frequency markFreq, int fontHeight);
    void drawMarkerLine(QPainter *painter, int ycoord);
};

#endif // BANDMAPFREQDIAL_H
