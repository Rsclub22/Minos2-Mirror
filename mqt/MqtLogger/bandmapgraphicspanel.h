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
    void leftMouseButtonPressed(QPoint);
    void mouseDoubleClicked(QPoint);
    void zoomMap(bool);
    void nextSpot(bool, bool);
    void scrollMap(bool);

protected:
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
private slots:

};

#endif // BANDMAPGRAPHICSPANEL_H
