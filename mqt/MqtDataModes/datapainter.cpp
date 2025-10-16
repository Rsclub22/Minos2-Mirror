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
    QGraphicsTextItem(text), engineWindow(engineWindow), row(r)
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
        trace("Right mouse press");
        // RMB was crashing deep within Qt, with no relationship
        // to this code.
        // It looks like a Qt bug. This solution comes from
        // https://stackoverflow.com/questions/67264846/pyqt5-program-crashes-when-editable-qgraphicstextitem-is-clicked-with-right-mo
        setTextInteractionFlags(Qt::NoTextInteraction);
    }
    else if (event && event->button() == Qt::LeftButton)
    {
        trace("Left mouse press");
        setTextInteractionFlags(Qt::TextEditable);
        QTextCursor c = textCursor();

        int cp = document()->documentLayout()->hitTest(event->pos(), Qt::FuzzyHit);

        // row is part of this DPGraphicsTextItem

//        QTextCursor is modeled on the way a text cursor behaves in a text editor,
//        providing a programmatic means of performing standard actions through the
//        user interface.
//
//        A document can be thought of as a single string of characters.
//
//        The cursor's current position() then is always either between two consecutive characters in the string,
//        or else before the very first character or after the very last character in the string.
//
//        Documents can also contain tables, lists, images, and other objects in addition to text but,
//        from the developer's point of view, the document can be treated as one long string.
//        Some portions of that string can be considered to lie within particular blocks (e.g. paragraphs),
//        or within a table's cell, or a list's item, or other structural elements.

//        When we refer to
//        "current character" we mean the character immediately before the cursor position() in the document.
//        Similarly, the "current block" is the block that contains the cursor position().

//        A QTextCursor also has an anchor() position. The text that is between the
//        anchor() and the position() is the selection.
//        If anchor() == position() there is no selection.

        // we have a problem when we have single numerics
        // works if we click before it, but not afterwards
        // Should we try backwards if sel char is a space?

        // walk back looking for a separator
        RXChar basec = engineWindow->rxBuff.getCharAt(row, cp);
        RXChar selc = basec;

        if (selc.getCh() == ' ')
        {
            if (cp > 0)
            {
                RXChar backc = engineWindow->rxBuff.getCharAt(row, cp -1);
                if (backc.getCh() != ' ')
                {
                    cp -= 1;
                    basec = engineWindow->rxBuff.getCharAt(row, cp);
                    selc = basec;
                }
                else
                {
                    c.setPosition(cp, QTextCursor::MoveAnchor);
                    setTextCursor(c);
                    return;
                }
            }
            else
            {
                c.setPosition(cp, QTextCursor::MoveAnchor);
                setTextCursor(c);
                return;
            }
        }
        while (cp > 0 && selc.isType(basec))
        {
            // Staying within word type, walk back one
            cp--;
            selc = engineWindow->rxBuff.getCharAt(row, cp);
        }
        if (!selc.isType(basec) && cp < engineWindow->rxBuff.getRxLine(row)->charCount())
        {
            // if that takes us out of word, go forwards one
            cp++;
            selc = engineWindow->rxBuff.getCharAt(row, cp);
        }
        // and set the cursor position at the start of the word
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
        trace(QString("Word selected is <%1>").arg(sel));
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
    ti->setTextInteractionFlags( Qt::TextEditorInteraction);
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
    // we really want to preserve selection as well
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
                if (nc.getCh() == ' ')
                {
                    rxbuff.append("&nbsp;");
                }
                else
                {
                    rxbuff.append(nc.getCh());
                }
            }
            QTextCursor cursor = lines[i]->textCursor();
            int startPos = cursor.selectionStart();
            int endPos = cursor.selectionEnd();
            QTextCharFormat tcf;
            cursor.setCharFormat(tcf);

            QFont cf = QApplication::font();

            lines[i]->setFont(cf);


            lines[i]->setHtml(rxbuff);

            if (startPos != endPos && startPos < cols && endPos < cols)
            {
                cursor.setPosition(startPos, QTextCursor::MoveAnchor);
                cursor.setPosition(endPos, QTextCursor::KeepAnchor);
                lines[i]->setTextCursor(cursor);
            }
            engineWindow->rxBuff.getRxLine(i)->setDirty(false);
        }
    }
}

DPScene::DPScene(QObject *parent):QGraphicsScene(parent)
{

}

