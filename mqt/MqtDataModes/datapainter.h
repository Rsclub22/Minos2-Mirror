#ifndef DATAPAINTER_H
#define DATAPAINTER_H

#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QStringList>
#include <QFont>

class EngineWindow;

class DPGraphicsTextItem: public QGraphicsTextItem
{
    Q_OBJECT
public:
    explicit DPGraphicsTextItem(QGraphicsItem *parent = nullptr);
    explicit DPGraphicsTextItem(EngineWindow *e, const QString &text, int r, QGraphicsItem *parent = nullptr);
    virtual ~DPGraphicsTextItem();
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

protected:

    EngineWindow *engineWindow = nullptr;
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
    void initialise(EngineWindow *e);
    EngineWindow *engineWindow = nullptr;

    void setText();

signals:

    void wordSelected(QString, int);

private:
    QGraphicsScene *scene = nullptr;
    QVector<QGraphicsTextItem *> lines;

    DPGraphicsTextItem *createNewLine(int r, int yoffset);
};

#endif // DATAPAINTER_H
