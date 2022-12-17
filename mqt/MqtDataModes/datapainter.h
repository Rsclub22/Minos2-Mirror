#ifndef DATAPAINTER_H
#define DATAPAINTER_H

#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QStringList>
#include <QFont>


class DPGraphicsTextItem: public QGraphicsTextItem
{
    Q_OBJECT
public:
    explicit DPGraphicsTextItem(QGraphicsItem *parent = nullptr);
    explicit DPGraphicsTextItem(const QString &text, QGraphicsItem *parent = nullptr);
    virtual ~DPGraphicsTextItem();

protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:

    void wordSelected(QString);

};

class DataPainter : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DataPainter(QWidget *parent = nullptr);
    virtual ~DataPainter()
    {}

    void setText();

signals:

    void wordSelected(QString);

private:
    QGraphicsScene *scene;
    QVector<QGraphicsTextItem *> lines;
    QFont ff;

};

#endif // DATAPAINTER_H
