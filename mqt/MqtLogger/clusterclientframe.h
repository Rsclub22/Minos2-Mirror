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
#include "clusterrpc.h"
//#include "MinosLoggerEvents.h"
#include "LoggerContest.h"
namespace Ui {
    class ClusterClientFrame;
}


class ClusterClientFrame : public QFrame
{
    Q_OBJECT

    void traceMsg(QString msg);
public:
    explicit ClusterClientFrame(QWidget *parent);



    void setContest( BaseContestLog *ct );


private:
    Ui::ClusterClientFrame *ui;
    LoggerContestLog *ct = nullptr;

    DxSpotDataModel* dxSpotDataModel;
    QTableView* dxSpotView;
    void restoreDxSpotViewColumns();
};

#endif // CLUSTERCLIENTFRAME_H
