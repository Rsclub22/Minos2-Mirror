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
#include <QFrame>
#include "dxspotdatamodel.h"
#include "base_pch.h"
#include "clusterClientServer.h"
#include "clusterclientfilterdialog.h"
#include "clustercommon.h"
#include "contest.h"
#include "ContestApp.h"
#include "MinosLoggerEvents.h"
#include "bandmapcallsignmarker.h"
//#include "bandmap.h"
#include "bandmapview.h"
#include "bandmapfreqdial.h"
#include "bandmapdatamodel.h"
#include "bandmapclientfilterdialog.h"



namespace Ui {
    class BandmapClientFrame;
}


class BMP_MouseInObject;

class BandmapClientFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BandmapClientFrame(QWidget* parent);
    ~BandmapClientFrame() override;

    QTimer* mouseInFrameTimer;

    void setFreq(QString);
    void setContest(BaseContestLog *c);
    void setHoldUpdateFlag(bool state);
    bool isSpotQueueEmpty();
    void buttonHandleDxSpots();
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseTimerCheckNewSpots();
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


    QTimer* purgeTimer;
    QTimer* checkNewSpotsTimer;
    QTimer* checkNewFilters;


    BMP_MouseInObject* actionInObject;


    // cluster spots
    QVector<QString> spotQueue;

    BandmapView *bandmapView;
    QItemSelectionModel *selectionModel;
    QGraphicsView* bandmapGraphicsView;

    QSortFilterProxyModel* bandmapSpotProxyModel;
    BandmapClientFilterDialog* filterSetup;

    QString sfreq;
    double curFreq = 0;
    int mapViewHeight = 0;

    BandmapDataModel *bandmapDataModel;
    BandmapData *bandmapData;

    QMenu* spotsMenu;
    QAction* freqAction;
    QAction* bearingAction;
    QAction* logAction;
    QAction* memoryAction;
    QAction* clearSpotAction;
    BandmapData selectedSpotData;
    int selectedSpotRowNum;




    bool purgeSpotFlag;
    bool holdUpdateFlag;
    qlonglong timeToLive;

    int getBandOffSet(QString contestBandStr);
    int getModeOffSet(QString contestModeStr);
    void handleDxSpots(QVector<QString> &spotQueue);
    void handleClusterStatusMessage(QString &msg);
    void statusIndicatorToggle(bool on);
    void addDxSpotToBandmapTable(const QString spot);
    void calcSpotDistanceBearing(const QString &_locator, double *distance, int *bearing);
    void checkSpotWorked(QString &callsign, QString &locator, bool *callWorked, bool *locatorWorked);
    void checkSpotInTable(QStringList &sl);
    void sendFreqToRig(QString freq);


    bool event(QEvent *event) override;
protected:


private slots:

     void clusterClientServerList(QVector<ClusterServer>);
     void dxSpots(QVector<QString>);


     void on_FontChanged();

     void checkNewBandMapSpots();

     void on_contextMenuSelected(const QPoint &pos);
     void on_freqActionSelected();
     void bearingActionSelected();
     void logActionSelected();
     void memoryActionSelected();
     void clearSpotActionSelected();
     void sendBrgToRot(QString brg);
     void on_AfterLogContact(BaseContestLog *c, Callsign cs, QString loc, QString brg, QString freq);
     void filterButtonSelected();
     void checkSavedFilters();
     void onMenuShow();

     void on_FitersChanged(bool state);
     void purgeSpots();
};



class BMP_MouseInObject : public QObject
{
    Q_OBJECT
public:
    BMP_MouseInObject(QWidget *parent, BandmapClientFrame* frame)
    {
        Q_UNUSED(parent)
        bandmapFrame = frame;
    }




    bool eventFilter(QObject *obj, QEvent *event)
    {


        if (event->type() == QEvent::Enter)
     {
            bandmapFrame->setHoldUpdateFlag(true);
        }
        else if (event->type() == QEvent::Leave)
        {
            bandmapFrame->mouseInFrameTimer->stop();
            if (!bandmapFrame->isSpotQueueEmpty())
            {
                bandmapFrame->buttonHandleDxSpots();
            }
            bandmapFrame->setHoldUpdateFlag(false);

        }

        return QObject::eventFilter(obj, event);
    }



private:

BandmapClientFrame* bandmapFrame;


};



#endif // BANDMAPCLIENTFRAME_H
