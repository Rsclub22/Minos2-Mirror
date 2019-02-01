////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef BANDMAPCLIENTFRAME_H
#define BANDMAPCLIENTFRAME_H

#include <QObject>
#include <QGraphicsScene>
#include <QFrame>

#include "bandmapcallsignmarker.h"
#include "bandmap.h"
#include "bandmapfreqdial.h"

namespace Ui {
    class BandmapClientFrame;
}



class BandmapClientFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BandmapClientFrame(QWidget* parent);
    ~BandmapClientFrame() override;



private:

    Ui::BandmapClientFrame *ui;
    QGraphicsScene *bandmapScene;
    BandmapFreqDial *dial;
    Bandmap *bandmap;

    QString sfreq;
    double curFreq = 0;
    int mapViewHeight = 0;


protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;


private slots:
     void drawDial(double frequency);


};

#endif // BANDMAPCLIENTFRAME_H
