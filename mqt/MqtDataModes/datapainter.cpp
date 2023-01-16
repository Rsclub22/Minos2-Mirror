#include <QPainter>
#include <QApplication>
#include <QGraphicsTextItem>
#include <QTextCursor>
#include <QMouseEvent>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QGraphicsSceneEvent>

#include <QStyleOption>

#include "rxbuffer.h"
#include "datapainter.h"

// Each line of the display is one of these

DPGraphicsTextItem::DPGraphicsTextItem(QGraphicsItem *parent):
    QGraphicsTextItem(parent), row(-1)
{}

DPGraphicsTextItem::DPGraphicsTextItem(const QString &text, int r, QGraphicsItem *parent):
    QGraphicsTextItem(text, parent), row(r)
{}

DPGraphicsTextItem::~DPGraphicsTextItem()
{}
void DPGraphicsTextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QStyleOptionGraphicsItem opt(*option);

    // Remove the selection style state, to prevent the dotted line from being drawn.
    opt.state = QStyle::State_None;

     // Call the parent to do the actual text drawing
    QGraphicsTextItem::paint(painter, &opt, widget);
}

void DPGraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QTextCursor c = textCursor();

    int cp = document()->documentLayout()->hitTest(event->pos(), Qt::FuzzyHit);
    c.setPosition(cp);
    c.select(QTextCursor::WordUnderCursor);
    setTextCursor(c);

    int cfreq = 0;
    if (cp < RxBuffer::getRxBuffer()->getRxLine(row)->size())
    {
        cfreq = RxBuffer::getRxBuffer()->getCharAt(row, cp).getCarrier();
    }


    QString sel = c.selectedText();
    if (!sel.isEmpty())
    {
        emit wordSelected(c.selectedText(), cfreq);
    }
}

//==========================================================================
DataPainter::DataPainter(QWidget *parent)
    : QGraphicsView{parent}
{
    scene = new QGraphicsScene(this);
    setScene(scene);

    QFont cf = QApplication::font();

    ff = QFont("Courier", cf.pointSize());
    scene->setFont(ff);

    QFontMetrics fm(ff);
    int h = fm.height();

    int yoffset = 0;

    int l = RxBuffer::getRxBuffer()->getLines();
    for (int i = 0; i < l; i++)
    {
        DPGraphicsTextItem *ti =  new DPGraphicsTextItem(QString(), i);
        connect(ti, &DPGraphicsTextItem::wordSelected, this, [this](QString s, int carr)
                {emit wordSelected(s, carr);});
        ti->setFont(ff);
        scene->addItem(ti);
        ti->setPos(0, yoffset);
        ti->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextEditable | Qt::TextEditorInteraction);
        ti->setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable | ti->flags());
        lines.push_back(ti);
        yoffset += h;
    }
}

void DataPainter::setText()
{
    int nlines = RxBuffer::getRxBuffer()->getLines();
    for (int i = 0; i < nlines; i++)
    {
        if (RxBuffer::getRxBuffer()->getRxLine(i)->getDirty())
        {
            QString rxbuff;
            int cols = RxBuffer::getRxBuffer()->getCols(i);
            for (int j = 0; j < cols; j++)
            {
                RXChar nc = RxBuffer::getRxBuffer()->getCharAt(i, j);
                rxbuff.append(nc.getCh());
            }
            QTextCharFormat tcf;
            tcf.setFont(ff);
            lines[i]->textCursor().setCharFormat(tcf);
            lines[i]->setHtml(rxbuff);

            RxBuffer::getRxBuffer()->getRxLine(i)->setDirty(false);
        }
    }
}

