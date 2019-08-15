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
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // BANDMAPGRAPHICSPANEL_H
