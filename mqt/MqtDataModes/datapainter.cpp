#include <QPainter>
#include <QApplication>
#include <QGraphicsTextItem>
#include <QTextCursor>
#include <QMouseEvent>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QGraphicsSceneEvent>

#include <QStyleOption>

#include "MTrace.h"
#include "enginewindow.h"
#include "cutils.h"
#include "rxbuffer.h"
#include "datapainter.h"

// Each line of the display is one of these

//DPGraphicsTextItem::DPGraphicsTextItem(QGraphicsItem *parent):
//    QGraphicsTextItem(parent), row(-1)
//{}

DPGraphicsTextItem::DPGraphicsTextItem(EngineWindow *engineWindow, const QString &text, int r):
    QGraphicsTextItem(text), row(r), engineWindow(engineWindow)
{
    //setAcceptedMouseButtons(Qt::LeftButton);
}

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
    trace("mouse press");
    if (event->button() == Qt::RightButton)
    {
        // This looks like a Qt bug. Solution comes from
        // https://stackoverflow.com/questions/67264846/pyqt5-program-crashes-when-editable-qgraphicstextitem-is-clicked-with-right-mo
        setTextInteractionFlags(Qt::NoTextInteraction);
    }
    else if (event && event->button() == Qt::LeftButton)
    {
        trace("Left mouse press");
        setTextInteractionFlags(Qt::TextEditable);
        QTextCursor c = textCursor();

        int cp = document()->documentLayout()->hitTest(event->pos(), Qt::FuzzyHit);

        // walk back and forward looking for a separator
        RXChar basec = engineWindow->rxBuff.getCharAt(row, cp);
        RXChar selc = basec;
        if (selc.getCh() == ' ')
        {
            c.setPosition(cp, QTextCursor::MoveAnchor);
            setTextCursor(c);
            return;
        }
        while (cp > 0 && selc.isType(basec))
        {
            cp--;
            selc = engineWindow->rxBuff.getCharAt(row, cp);
        }
        if (!selc.isType(basec) && cp < engineWindow->rxBuff.getRxLine(row)->charCount())
        {
            cp++;
            selc = engineWindow->rxBuff.getCharAt(row, cp);
        }
        c.setPosition(cp, QTextCursor::MoveAnchor);

        // and forwards
        while (cp < engineWindow->rxBuff.getRxLine(row)->charCount()  && selc.isType(basec))
        {
            cp++;
            if (cp < engineWindow->rxBuff.getRxLine(row)->charCount())
            {
                selc = engineWindow->rxBuff.getCharAt(row, cp);
            }
        }

        c.setPosition(cp, QTextCursor::KeepAnchor);
        setTextCursor(c);

        int mfreq = 0;
        if (cp < engineWindow->rxBuff.getRxLine(row)->charCount())
        {
            mfreq = engineWindow->rxBuff.getCharAt(row, cp).getMarkFreq();
        }


        QString sel = c.selectedText();
        if (!sel.isEmpty())
        {
            emit wordSelected(sel, mfreq);
        }
    }
}

//==========================================================================
DPGraphicsTextItem * DataPainter::createNewLine(int r, int yoffset)
{
    DPGraphicsTextItem *ti =  new DPGraphicsTextItem(engineWindow, QString(), r);
    connect(ti, &DPGraphicsTextItem::wordSelected, this, [this](QString s, int mfreq)
            {emit wordSelected(s, mfreq);});

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

    scene = new DPScene(this);
    setScene(scene);

    QFont cf = QApplication::font();
    QFontMetrics fm(cf);
    int ls = fm.lineSpacing();
    int h = ls *3/2;

    int yoffset = 0;

    int l = engineWindow->rxBuff.getLines();
    for (int i = 0; i < l; i++)
    {
        DPGraphicsTextItem *ti =  createNewLine(i, yoffset);
        scene->addItem(ti);
        lines.push_back(ti);

        if (!ti->isActive())
        {
            ti->setActive(true);
        }

        yoffset += h;
    }
}

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
            QColor colourSet = Qt::black;
            bool bold = false;
            bool boldSet = false;
            for (int j = 0; j < cols; j++)
            {
                RXChar nc = engineWindow->rxBuff.getCharAt(i, j);
                if (nc.getMyCall())
                {
                    colour = Qt::darkRed;
                    bold = true;
                }
                else if (nc.getRST())
                {
                    colour = Qt::darkYellow;
                    bold = false;
                }
                else if (nc.getSerial())
                {
                    colour = Qt::darkGreen;
                    bold = true;
                }
                else if (nc.getWorkedCall())
                {
                    colour = Qt::gray;
                    bold = false;
                }
                else if (nc.getUnworkedCall())
                {
                    colour = Qt::blue;
                    bold = true;
                }
                else
                {
                    colour = Qt::black;
                    bold = false;
                }

                if (colour != Qt::black)
                {
                    if (nc.getCh() == QChar(' '))
                    {
                        colour =Qt::black;
                        bold = false;
                    }
                }
                if (bold != boldSet && bold)
                {
                    rxbuff.append("<b>");
                    boldSet = bold;
                }
                if (colour != colourSet)
                {
                    QString cstr = HtmlFontColour(colour);
                    rxbuff.append(cstr);
                    colourSet = colour;
                }
                if (bold != boldSet && !bold)
                {
                    rxbuff.append("</b>");
                    boldSet = bold;
                }

                rxbuff.append(nc.getCh());
            }
            QTextCharFormat tcf;
            lines[i]->textCursor().setCharFormat(tcf);

            QFont cf = QApplication::font();

            lines[i]->setFont(cf);


            lines[i]->setHtml(rxbuff);

            engineWindow->rxBuff.getRxLine(i)->setDirty(false);
        }
    }
}

DPScene::DPScene(QObject *parent):QGraphicsScene(parent)
{

}

