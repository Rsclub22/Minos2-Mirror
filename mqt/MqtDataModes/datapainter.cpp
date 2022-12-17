#include <QPainter>
#include <QApplication>
#include <QGraphicsTextItem>
#include <QTextCursor>
#include <QMouseEvent>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QGraphicsSceneEvent>

#include "rxbuffer.h"
#include "datapainter.h"

// Each line of the display is one of these

DPGraphicsTextItem::DPGraphicsTextItem(QGraphicsItem *parent):
    QGraphicsTextItem(parent)
{}

DPGraphicsTextItem::DPGraphicsTextItem(const QString &text, QGraphicsItem *parent):
    QGraphicsTextItem(text, parent)
{}

DPGraphicsTextItem::~DPGraphicsTextItem()
{}

void DPGraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QTextCursor c = textCursor();

    int cp = document()->documentLayout()->hitTest(event->pos(), Qt::FuzzyHit);
    c.setPosition(cp);
    c.select(QTextCursor::WordUnderCursor);
    setTextCursor(c);

    QString sel = c.selectedText();
    if (!sel.isEmpty())
    {
        emit wordSelected(c.selectedText());
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
        DPGraphicsTextItem *ti =  new DPGraphicsTextItem(QString());
        connect(ti, &DPGraphicsTextItem::wordSelected, this, [this](QString s){emit wordSelected(s);});
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

