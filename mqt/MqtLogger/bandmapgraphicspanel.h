#ifndef BANDMAPGRAPHICSPANEL_H
#define BANDMAPGRAPHICSPANEL_H

#include <QObject>
#include <QGraphicsView>

class BandmapGraphicsPanel : public QGraphicsView
{

    Q_OBJECT

public:
    BandmapGraphicsPanel(QWidget *parent);

signals:
    void bandmapResize(int);

protected:
    void resizeEvent(QResizeEvent *) override;
};

#endif // BANDMAPGRAPHICSPANEL_H
