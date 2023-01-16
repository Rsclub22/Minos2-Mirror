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
    explicit DPGraphicsTextItem(const QString &text, int r, QGraphicsItem *parent = nullptr);
    virtual ~DPGraphicsTextItem();
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

protected:

    int row = -1;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:

    void wordSelected(QString, int);

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

    void wordSelected(QString, int);

private:
    QGraphicsScene *scene;
    QVector<QGraphicsTextItem *> lines;
    QFont ff;

};

#endif // DATAPAINTER_H
