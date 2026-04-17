#include <QScrollBar>

#include "ScreenConfigElement.h"
#include "ScreenConfigScreen.h"
#include "ScreenConfig.h"
#include  "MTrace.h"

#include "ScreenConfigRow.h"
#include "minossplitter.h"
#include "ui_ScreenConfigRow.h"

ScreenConfigRow::ScreenConfigRow(ScreenConfigElement *parentc) :
    QFrame(nullptr)
  , ui(new Ui::ScreenConfigRow)
  , parentElement(parentc)
{
    static int rollingTag = 0;
    tag = rollingTag++;
    ui->setupUi(this);
    QHBoxLayout *hbl = new QHBoxLayout(ui->scrollAreaWidgetContents);
    ui->scrollAreaWidgetContents->setLayout(hbl);

    hmsplit = new MinosSplitter(ui->scrollAreaWidgetContents);
    hmsplit->setOrientation(Qt::Horizontal);
    hmsplit->setContentsMargins(0, 0, 0, 0);
    hbl->addWidget(hmsplit);

    setStyleSheet("background-color: light grey;");

    // colours from https://www.december.com/html/spec/colorsvg.html
//    setStyleSheet("#ScreenConfigRow { border: 2px solid darkcyan; }");
//    setStyleSheet("#ScreenConfigRow { border: 1px solid blue; }");

    setContentsMargins(0, 0, 0, 0);
}

ScreenConfigRow::~ScreenConfigRow()
{
    delete ui;
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
    for (int i = 0; i < hmsplit->count(); i++)
    {
        if (hmsplit->widget(i) == e)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigElement *baseElement = new ScreenConfigElement(this);
    hmsplit->insertWidget( pos, baseElement);

    return baseElement;
}
void ScreenConfigRow::remove(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < hmsplit->count(); i++)
    {
        if (hmsplit->widget(i) == e)
        {
            pos = i;
            break;
        }
    }
    QObject *taken = hmsplit->widget(pos);
    if (taken)
    {
        delete taken;
    }
    if (hmsplit->count() == 0)
    {
        parentElement->removeRow(this);
    }
    else if (hmsplit->count() == 1)
    {
        QObject *w = hmsplit->widget(0);
        ScreenConfigElement *ele = dynamic_cast<ScreenConfigElement *>(w);
        if (ele->getIsSplitElement())
        {
            unsplit();
        }
    }
}
void ScreenConfigRow::unsplit()
{
    // we need to take the rows of the split element, and make them this rows rows
    // we need to put in the same place as this row, in order

    // "this" is the current row, containing split element and its rows
    // parent element should be the next level up

    int prowno = -1;
    int ct = (parentElement->vmsplit)?parentElement->vmsplit->count():0;
    ScreenConfigRow *rl = nullptr;
    QWidget *pw = nullptr;
    QObject *wl = nullptr;
    ScreenConfigElement *pelement = nullptr;
    for (int i = 0; i < ct; i++)
    {
        wl = parentElement->vmsplit->widget(i);
        if (wl == this)
        {
            rl = dynamic_cast<ScreenConfigRow *>(wl);
            pw = rl->parentWidget();
            pelement = rl->parentElement;
            prowno = i;
            break;
        }
    }

    QObject *we = hmsplit->widget(0);
    ScreenConfigElement *splitele = dynamic_cast<ScreenConfigElement *>(we);    // and this is its split widget

    int inc = 1;
    if (splitele->vmsplit)
    {
        while (QObject *l = splitele->vmsplit->widget(0)) // ele->parentElement
        {
            l->setParent(nullptr);
            QWidget *w = dynamic_cast<QWidget *>(l);
            parentElement->vmsplit->insertWidget(prowno + inc, w);
            ScreenConfigRow *scr = dynamic_cast<ScreenConfigRow *>(w);
            if (scr)
            {
                scr->parentElement = pelement;
            }
            w->setParent(pw);
            w->setVisible(true);        // why isn't it visible?
            inc++;

        }
    }
    // and get rid of the split element/row
    if (parentElement->vmsplit)
    {
        QObject *item = parentElement->vmsplit->widget(prowno);
        delete item;
    }
}
ScreenConfigElement *ScreenConfigRow::addRight(ScreenConfigElement *e)
{
    int pos = 0;
    int offset = 0;
    for (int i = 0; i < hmsplit->count(); i++)
    {
        if (hmsplit->widget(i) == e)
        {
            pos = i;
            offset = 1;
            break;
        }
    }
    if (e == nullptr && hmsplit->count() > 0)
    {
        offset = 1;
    }
    ScreenConfigElement *baseElement = new ScreenConfigElement(this);
    hmsplit->insertWidget( pos + offset, baseElement);

    return baseElement;
}

bool ScreenConfigRow::checkOk(ScreenConfigElement *e)
{
    return parentElement->checkOk(e);
}

void ScreenConfigRow::mousePressEvent(QMouseEvent *)
{
    QVector<ScreenConfigRow *> sel = screenConfigDialog->curScreen->getSelected();
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
