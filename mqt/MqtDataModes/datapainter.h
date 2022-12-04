#ifndef DATAPAINTER_H
#define DATAPAINTER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QStringList>
#include <QFont>


class QGraphicsTextItem;

class DataPainter : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DataPainter(QWidget *parent = nullptr);
    virtual ~DataPainter()
    {}

    void setText();

signals:

private:
    QGraphicsScene *scene;
    QVector<QGraphicsTextItem *> lines;
    QFont ff;

};

#endif // DATAPAINTER_H
