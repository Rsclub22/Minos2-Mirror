#include <QPainter>
#include <QApplication>
#include <QGraphicsTextItem>
#include <QTextCursor>

#include "rxbuffer.h"
#include "datapainter.h"


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
        QGraphicsTextItem *ti = scene->addText(QString());
        ti->setPos(0, yoffset);
        ti->setTextInteractionFlags(Qt::TextEditorInteraction | Qt::TextSelectableByMouse);
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
