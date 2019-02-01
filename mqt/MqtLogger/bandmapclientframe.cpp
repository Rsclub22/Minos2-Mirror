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

#include "base_pch.h"
#include "cutils.h"
#include <QDebug>
#include "ui_bandmapclientframe.h"

#include "bandmapclientframe.h"

BandmapClientFrame::BandmapClientFrame(QWidget *parent):
    QFrame(parent),
    ui(new Ui::BandmapClientFrame)
{

    ui->setupUi(this);

    bandmapScene = new QGraphicsScene(this);
    ui->bandmapView->setScene(bandmapScene);
    ui->bandmapView->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    bandmap = new Bandmap();
    dial = new BandmapFreqDial();
    bandmapScene->addItem(dial);

}


BandmapClientFrame::~BandmapClientFrame()
{
    delete ui;
}



void BandmapClientFrame::drawDial(double frequency)
{


    if (!almost_equal(frequency, dial->getCurFreq(), 2))
    {
        dial->setCurFreq(frequency);
        dial->setCurHeight(ui->bandmapView->height());
        dial->update();
    }

}


void BandmapClientFrame::resizeEvent(QResizeEvent *event)
{
    mapViewHeight = ui->bandmapView->height() - 2;
    if (dial->getCurHeight() != mapViewHeight)
    {
        dial->changeBoundingRect(mapViewHeight);
        bandmapScene->setSceneRect(bandmapScene->itemsBoundingRect());
    }
    qDebug() << "view height" << mapViewHeight;
    qDebug() << "dial height" << dial->boundingRect().height();

    dial->setCurFreq(curFreq);
    //dial->setCurHeight(mapViewHeight);
    dial->update();

    QWidget::resizeEvent(event);
}

void BandmapClientFrame::setContest(BaseContestLog *c)
{
    //ct = c;
    //LoggerContestLog* contest = dynamic_cast<LoggerContestLog *>( ct);
}

void BandmapClientFrame::paintEvent(QPaintEvent  * /*event */)
{

}
