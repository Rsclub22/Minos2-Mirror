#include "bandmapgraphicspanel.h"
#include <QDebug>

BandmapGraphicsPanel::BandmapGraphicsPanel(QWidget *parent)
{

}


void BandmapGraphicsPanel::resizeEvent(QResizeEvent *)
{

    qDebug() << "resize height " << size();
    emit bandmapResize(size().height());

}
