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
    void bandmapResize(QSize);
    void leftMouseButtonPressed(QPoint);
    void mouseDoubleClicked(QPoint);

protected:
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
private slots:

};

#endif // BANDMAPGRAPHICSPANEL_H
