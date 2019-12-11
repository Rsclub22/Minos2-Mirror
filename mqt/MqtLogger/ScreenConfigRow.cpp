#include "base_pch.h"
#include <QScrollBar>

#include "ScreenConfigElement.h"
#include "ScreenConfigRow.h"
#include "ScreenConfig.h"

#include "ui_ScreenConfigRow.h"

ScreenConfigRow::ScreenConfigRow(ScreenConfigElement *parentc) :
    QFrame(nullptr)
  , ui(new Ui::ScreenConfigRow)
  , parentElement(parentc)
{
    ui->setupUi(this);
    hbl = new QHBoxLayout(ui->scrollAreaWidgetContents);
    hbl->setMargin(1);
    ui->scrollAreaWidgetContents->setLayout(hbl);

    setStyleSheet("background-color: light grey;");
    setStyleSheet("#ScreenConfigRow { border: 2px solid darkcyan; }");
}

ScreenConfigRow::~ScreenConfigRow()
{
    delete ui;
}

QScrollArea *ScreenConfigRow::getScrollArea()
{
    return ui->scrollArea;
}

void ScreenConfigRow::on_addRowBeforeButton_clicked()
{
    trace("ScreenConfigRow::on_addRowBeforeButton_clicked()");
    parentElement->addRowBefore(this);
}

void ScreenConfigRow::on_removeRowButton_clicked()
{
    trace("ScreenConfigRow::on_removeRowButton_clicked()");
    parentElement->removeRow(this);
}

void ScreenConfigRow::on_addRowAfterButton_clicked()
{
    trace("ScreenConfigRow::on_addRowAfterButton_clicked()");
    parentElement->addRowAfter(this);
}

ScreenConfigElement * ScreenConfigRow::addLeft(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < hbl->count(); i++)
    {
        if (hbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigElement *baseElement = new ScreenConfigElement(this);
    hbl->insertWidget( pos, baseElement);

    return baseElement;
}
void ScreenConfigRow::remove(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < hbl->count(); i++)
    {
        if (hbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    QLayoutItem *taken = hbl->takeAt(pos);
    if (taken)
    {
        // From the source, I don't think the deleting the layout item deletes the widget
        taken->widget()->deleteLater();
        delete taken;
    }
    if (hbl->count() == 0)
    {
        parentElement->removeRow(this);
    }
    else if (hbl->count() == 1)
    {
        QWidget *w = hbl->itemAt(0)->widget();
        ScreenConfigElement *ele = dynamic_cast<ScreenConfigElement *>(w);
        if (ele->getIsSplitElement())
        {
            unsplit();
        }
    }
}
//ScreenConfigRow *ScreenConfig::combineRows(int top, int bottom)
//{
//    QWidget *qli = vbl->itemAt(top)->widget();
//    ScreenConfigRow *scr = dynamic_cast<ScreenConfigRow *>(qli);

//    scr->parentElement->addRowBefore(scr);
//    ScreenConfigRow *newRow = dynamic_cast<ScreenConfigRow *>(scr->parentElement->vbl->itemAt(top)->widget());

//    //new row needs a split element, to which we add our old rows

//    QWidget *w = newRow->hbl->itemAt(0)->widget();
//    ScreenConfigElement *split = dynamic_cast<ScreenConfigElement *>(w);
//    split->setIsSplitElement(true);
//    split->setType(sctSplit);

//    for (int i = top + 1; i <= bottom + 1; i++)
//    {
//        // keep taking the top of the old, and put it back at the bottom of the new
//        QLayoutItem *l = scr->parentElement->vbl->takeAt(top + 1);

//        split->vbl->addItem(l);
//        // reset the parentage, or it all displays in the wrong place
//        l->widget()->setParent(split);
//    }
//    return  newRow;
//}
void ScreenConfigRow::unsplit()
{
    // we need to take the rows of the split element, and make them this rows rows
    // we need to put in the same place as this row, in order

    // "this" is the current row, containing split element and its rows
    // parent element should be the next level up

    int prowno = -1;
    int ct = parentElement->vbl->count();
    ScreenConfigRow *rl = nullptr;
    QWidget *pw = nullptr;
    QWidget *wl = nullptr;
    ScreenConfigElement *pelement = nullptr;
    for (int i = 0; i < ct; i++)
    {
        wl = parentElement->vbl->itemAt(i)->widget();
        if (wl == this)
        {
            rl = dynamic_cast<ScreenConfigRow *>(wl);
            pw = rl->parentWidget();
            pelement = rl->parentElement;
            prowno = i;
            break;
        }
    }

    QWidget *we = hbl->itemAt(0)->widget();
    ScreenConfigElement *splitele = dynamic_cast<ScreenConfigElement *>(we);    // and this is its split widget

    int inc = 1;
    while (QLayoutItem *l = splitele->vbl->takeAt(0)) // ele->parentElement
    {
        parentElement->vbl->insertItem(prowno + inc, l);
        QWidget *w = l->widget();
        ScreenConfigRow *scr = dynamic_cast<ScreenConfigRow *>(w);
        scr->parentElement = pelement;
        w->setParent(pw);
        w->setVisible(true);        // why isn't it visible?
        inc++;
    }
    // and get rid of the split element/row (which is "this" so use deleteLater)
    QLayoutItem *item = parentElement->vbl->takeAt(prowno);
    QWidget* widget = item->widget();
    widget->deleteLater();
    delete item;
}
ScreenConfigElement *ScreenConfigRow::addRight(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < hbl->count(); i++)
    {
        if (hbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigElement *baseElement = new ScreenConfigElement(this);
    hbl->insertWidget( pos + 1, baseElement);

    return baseElement;
}

bool ScreenConfigRow::checkOk(ScreenConfigElement *e)
{
    return parentElement->checkOk(e);
}

void ScreenConfigRow::mousePressEvent(QMouseEvent *)
{
    QVector<ScreenConfigRow *> sel = screenConfigDialog->getSelected();
    if ( sel.count() == 0 || sel[0]->parentElement == parentElement)
    {
        mouseDown = true;
    }
}
void ScreenConfigRow::mouseReleaseEvent(QMouseEvent *)
{
    if (mouseDown)
    {
        if (selected)
        {
            setStyleSheet("background-color: light grey;");
            selected = false;
        }
        else
        {
            setStyleSheet("background-color: aqua;");
            selected = true;
        }
    }
    mouseDown = false;
}
void ScreenConfigRow::leaveEvent(QEvent *)
{
    if (mouseDown )
    {
        mouseDown = false;
    }
}
