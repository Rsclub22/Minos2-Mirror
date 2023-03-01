#include <QPainter>
#include <QApplication>
#include <QGraphicsTextItem>
#include <QTextCursor>
#include <QMouseEvent>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QGraphicsSceneEvent>

#include <QStyleOption>

#include "enginewindow.h"
#include "cutils.h"
#include "rxbuffer.h"
#include "datapainter.h"

// Each line of the display is one of these

DPGraphicsTextItem::DPGraphicsTextItem(QGraphicsItem *parent):
    QGraphicsTextItem(parent), row(-1)
{}

DPGraphicsTextItem::DPGraphicsTextItem(EngineWindow *engineWindow, const QString &text, int r, QGraphicsItem *parent):
    QGraphicsTextItem(text, parent), row(r), engineWindow(engineWindow)
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
    if (cp < engineWindow->rxBuff.getRxLine(row)->charCount())
    {
        cfreq = engineWindow->rxBuff.getCharAt(row, cp).getCarrier();
    }


    QString sel = c.selectedText();
    if (!sel.isEmpty())
    {
        emit wordSelected(c.selectedText(), cfreq);
    }
}

//==========================================================================
DPGraphicsTextItem * DataPainter::createNewLine(int r, int yoffset)
{
    DPGraphicsTextItem *ti =  new DPGraphicsTextItem(engineWindow, QString(), r);
    connect(ti, &DPGraphicsTextItem::wordSelected, this, [this](QString s, int carr)
            {emit wordSelected(s, carr);});
    ti->setFont(ff);
    scene->addItem(ti);
    ti->setPos(0, yoffset);
    ti->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextEditable | Qt::TextEditorInteraction);
    ti->setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable | ti->flags());
    return ti;
}
DataPainter::DataPainter(QWidget *parent)
    : QGraphicsView{parent}
{

}

void DataPainter::initialise(EngineWindow *e)
{
    engineWindow = e;

    scene = new QGraphicsScene(this);
    setScene(scene);

    QFont cf = QApplication::font();

    ff = QFont("Courier", cf.pointSize());
    scene->setFont(ff);

    QFontMetrics fm(ff);
    int h = fm.height();

    int yoffset = 0;

    int l = engineWindow->rxBuff.getLines();
    for (int i = 0; i < l; i++)
    {
        DPGraphicsTextItem *ti =  createNewLine(i, yoffset);
        lines.push_back(ti);
        yoffset += h;
    }}

void DataPainter::setText()
{
    int nlines = engineWindow->rxBuff.getLines();
    for (int i = 0; i < nlines; i++)
    {
        if (engineWindow->rxBuff.getRxLine(i)->getDirty())
        {
            QString rxbuff;
            int cols = engineWindow->rxBuff.getCols(i);
            QColor colour = Qt::black;
            for (int j = 0; j < cols; j++)
            {
                RXChar nc = engineWindow->rxBuff.getCharAt(i, j);
                if (nc.getMyCall())
                {
                    colour = Qt::darkRed;
                }
                else if (nc.getRST())
                {
                    colour = Qt::darkYellow;
                }
                else if (nc.getSerial())
                {
                    colour = Qt::darkGreen;
                }
                else if (nc.getWorkedCall())
                {
                    colour = Qt::gray;
                }
                else if (nc.getUnworkedCall())
                {
                    colour = Qt::blue;
                }
                if (colour != Qt::black)
                {
                    if (nc.getCh() == QChar(' '))
                    {
                        colour = Qt::black;
                    }
                    QString cstr = HtmlFontColour(colour);
                    rxbuff.append(cstr);
                }

                rxbuff.append(nc.getCh());
            }
            QTextCharFormat tcf;
            tcf.setFont(ff);
            lines[i]->textCursor().setCharFormat(tcf);
            lines[i]->setHtml(rxbuff);

            engineWindow->rxBuff.getRxLine(i)->setDirty(false);
        }
    }
}

