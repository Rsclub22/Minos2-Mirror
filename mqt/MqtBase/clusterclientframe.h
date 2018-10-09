////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef CLUSTERCLIENTFRAME_H
#define CLUSTERCLIENTFRAME_H

#include <QObject>
#include <QFrame>
#include "dxspotdatamodel.h"
#include "base_pch.h"
#include "clusterClientServer.h"
#include "MinosLoggerEvents.h"

namespace Ui {
    class ClusterClientFrame;
}


class ClusterClientFrame : public QFrame
{
    Q_OBJECT

    void traceMsg(QString msg);
public:
    explicit ClusterClientFrame(QWidget *parent);
    ~ClusterClientFrame() override;
    void setStandAlone();
    void setContest(BaseContestLog *contest);




private:
    Ui::ClusterClientFrame *ui;
    BaseContestLog *contest;


    DxSpotDataModel* dxSpotDataModel;
    QTableView* dxSpotView;
    void restoreDxSpotViewColumns();

private slots:


    void clusterClientServerList(QVector<ClusterServer>);
    void dxSpots(QVector<QString>);
    void on_FontChanged();
};

#endif // CLUSTERCLIENTFRAME_H
