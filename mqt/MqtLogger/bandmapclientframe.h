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
//#include <QGraphicsScene>
#include <QFrame>
#include "dxspotdatamodel.h"
#include "base_pch.h"
#include "clusterClientServer.h"
#include "clusterclientfilterdialog.h"
#include "clustercommon.h"
#include "contest.h"
#include "ContestApp.h"
#include "bandModeFrequencyPlan.h"
#include "MinosLoggerEvents.h"
#include "bandmapcallsignmarker.h"
#include "bandmap.h"
#include "bandmapfreqdial.h"
#include "bandmapdatamodel.h"

namespace Ui {
    class BandmapClientFrame;
}



class BandmapClientFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BandmapClientFrame(QWidget* parent);
    ~BandmapClientFrame() override;


    void setFreq(QString);
    void setContest(BaseContestLog *c);
private:

    Ui::BandmapClientFrame *ui;
    bool isProtected;
    BaseContestLog *ct = nullptr;
    QString contestUuid;
    QString contestBandStr;
    int contestBand;
    QString contestModeStr;
    int contestMode;

    QString lastfreq;


    // cluster spots
    QVector<QString> spotQueue;

    //QGraphicsScene *bandmapScene;
    //BandmapFreqDial *dial;
    Bandmap *bandmap;

    QString sfreq;
    double curFreq = 0;
    int mapViewHeight = 0;

    BandmapDataModel *bandmapDataModel;
    BandmapData *bandmapData;


    int getBandOffSet(QString contestBandStr);
    int getModeOffSet(QString contestModeStr);
    void handleDxSpots(QVector<QString> &spotQueue);
    void handleClusterStatusMessage(QString &msg);
    void statusIndicatorToggle(bool on);
protected:
    //void resizeEvent(QResizeEvent *event) override;
    //void paintEvent(QPaintEvent *) override;


private slots:
     //void drawDial(double frequency);
     void clusterClientServerList(QVector<ClusterServer>);
     void dxSpots(QVector<QString>);


     void on_FontChanged();
};

#endif // BANDMAPCLIENTFRAME_H
