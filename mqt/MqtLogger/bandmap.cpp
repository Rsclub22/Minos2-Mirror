////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////



#include "bandmap.h"
#include "bandmapview.h"
#include "ui_clusterclientframe.h"


const int Invalid = -1;





Bandmap::Bandmap(QWidget *parent)
    : QWidget(parent),
      //bandmapFrame(parent),
      selectedRow(Invalid),
      selectedColumn(Invalid)
{

}



void Bandmap::initBandmap(BandmapDataModel *bmDataModel, QGraphicsView* _bandmapGraphicsView)
{
    QFontMetrics fm(font());

    bandmapDataModel = bmDataModel;
    bandmapGraphicsView = _bandmapGraphicsView;
    bandmapGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
    bandmapGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
    //bandmapGraphicsView->viewport()->update();
    //qDebug() << "height " << bandmapGraphicsView->size();

    bandmapView = new BandmapView(this);


    //bandmapScrollArea = bMapScrollArea;
    //bandmapScrollArea->setBackgroundRole(QPalette::Light);
   // bandmapScrollArea->setWidget(bandmapView);
    //bandmapScrollArea->installEventFilter(bandmapView);

    //bandmapLayout = new QHBoxLayout;
    //bandmapLayout->addWidget(bandmapScrollArea);
    //bandmapLayout->setContentsMargins(0, 0, 0, 0);
    //bandmapLayout->setSpacing(0);
    //setLayout(bandmapLayout);

    connect(bandmapView, SIGNAL(clicked(const QModelIndex&)), this, SIGNAL(clicked(const QModelIndex&)));
}

void Bandmap::setSelectedRow(int row)
{
    selectedRow = row;
    bandmapView->update();
}

int Bandmap::getBandmapFrameHeight()
{
    return bandmapGraphicsView->viewport()->height();
}

int Bandmap::getBandmapFrameWidth()
{
    return bandmapGraphicsView->viewport()->width();
}

void Bandmap::setSelectedColumn(int column)
{
    selectedColumn = column;
    //header->update();
}

void Bandmap::setCurrentIndex(const QModelIndex &index)
{
    setSelectedRow(index.row());
    //setSelectedColumn(index.column());
    //int x = xOffsetForMiddleOfColumn(index.column());
    //int y = yOffsetForRow(index.row());
    //bandmapScrollArea->ensureVisible(x, y, 10, 20);
}





void Bandmap::setFreq(double freq)
{
    bandmapView->setFreq(freq);
}
