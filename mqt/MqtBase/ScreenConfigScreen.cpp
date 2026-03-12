#include <QScrollBar>

#include "MShowMessageDlg.h"
#include "ScreenConfigElement.h"
#include "ScreenConfigRow.h"
#include "ScreenConfig.h"
#include "MTrace.h"

#include "ScreenConfigScreen.h"
#include "minossplitter.h"
#include "ui_ScreenConfigScreen.h"

ScreenConfigScreen::ScreenConfigScreen(ScreenConfig *parentc) :
    QFrame(parentc),
    parentElement(parentc),
    ui(new Ui::ScreenConfigScreen)
{
    ui->setupUi(this);

    QVBoxLayout *vbl = new QVBoxLayout(ui->scrollAreaWidgetContents);
    vmsplit = new MinosSplitter(ui->scrollAreaWidgetContents);
    vmsplit->setOrientation(Qt::Vertical);
    vbl->addWidget(vmsplit);
    vmsplit->setContentsMargins(0, 0, 0, 0);
    ui->scrollAreaWidgetContents->setLayout(vbl);

    baseElement = new ScreenConfigElement(nullptr, this);

    setContentsMargins(0, 0, 0, 0);

}

ScreenConfigScreen::~ScreenConfigScreen()
{
    delete ui;
}

void ScreenConfigScreen::setName(QString n)
{
    name = n;
    ui->screenNameEdit->setText(n);
}
void ScreenConfigScreen::setNameFocus()
{
    ui->screenNameEdit->setFocus();
}
bool ScreenConfigScreen::checkRowOk(const ScreenConfigRow *row, ScreenConfigElement *e, int &auxCount)
{
    SCType etype = e->getType();
    if (row)
    {
        for (int j = 0; j < row->hmsplit->count(); j++)
        {
            const QObject *w = row->hmsplit->widget(j);
            const ScreenConfigElement *ele = dynamic_cast<const ScreenConfigElement *>(w);
            if (ele && ele != e)
            {
                SCType type = ele->getType();
                if (ele->getIsSplitElement() && ele->vmsplit)
                {
                    for (int i = 0; i < ele->vmsplit->count(); i++)
                    {
                        ScreenConfigRow *r = dynamic_cast<ScreenConfigRow *>(ele->vmsplit->widget(i));
                        if (!checkRowOk(r, e, auxCount))
                            return false;
                    }
                }
                else if (type == etype)
                {
                    if (type == sctAux)
                    {
                        auxCount++;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool ScreenConfigScreen::checkOk(ScreenConfigElement *e)
{
    return parentElement->checkOk(e);
}
bool ScreenConfigScreen::checkScreenOk(ScreenConfigElement *e, int auxCount)
{
    for (int i = 0; i < vmsplit->count(); i++)
    {
        const QObject *w = vmsplit->widget(i);
        const ScreenConfigRow *row = dynamic_cast<const ScreenConfigRow *>(w);
        if (!checkRowOk(row, e, auxCount))
            return false;

    }
    return true;
}
void ScreenConfigScreen::buildRows(QVector<SCRow> rows, ScreenConfigElement *bele, MinosSplitter *vmsplit)
{
    for (int j = 0; j < rows.count(); j++)
    {
        ScreenConfigRow *baseRow = new ScreenConfigRow(bele);
        vmsplit->addWidget(baseRow);
       for (int k = 0; k < rows[j].elements.count(); k++)
       {
           ScreenConfigElement *e = new ScreenConfigElement(baseRow);
           SCType sctype = rows[j].elements[k].type;
           e->setType(sctype);
           if (sctype == sctAux)
           {
               e->setAuxType(rows[j].elements[k].auxType);
           }
           else if (sctype == sctSplit)
           {
               e->setIsSplitElement(true);
               MinosSplitter *v = e->vmsplit;
               buildRows(rows[j].elements[k].rows, e, v);
           }
           baseRow->hmsplit->insertWidget(k, e);
       }
    }
}
void ScreenConfigScreen::procRow(ScreenConfigRow *row, SCRow &scrow)
{
    if (row != nullptr)
    {
        int eleCt = row->hmsplit->count();
        for (int j = 0; j < eleCt; j++)
        {
            const QObject *w = row->hmsplit->widget(j);
            const ScreenConfigElement *ele = dynamic_cast<const ScreenConfigElement *>(w);
            if (ele)
            {
                SCElement scele;
                if (ele->getIsSplitElement())
                {
                    scele.type = sctSplit;

                    int vCt = ele->vmsplit->count();
                    for (int i = 0; i < vCt; i++)
                    {
                        ScreenConfigRow *r = dynamic_cast<ScreenConfigRow *>(ele->vmsplit->widget(i));
                        if (r != nullptr)
                        {
                            SCRow scr;
                            procRow(r, scr);
                            scele.rows.push_back(scr);
                        }
                    }
                }
                else
                {
                    scele.type = ele->getType();
                    scele.auxType = ele->getAuxType();
                }
                scrow.elements.append(scele);
            }
        }
    }
}
void ScreenConfigScreen::getConfig(SCScreen &sc)
{
    name = ui->screenNameEdit->text();
    int vCt = vmsplit->count();
    for (int i = 0; i < vCt; i++)
    {
        QObject *w = vmsplit->widget(i);
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row)
        {
            SCRow scrow;
            procRow(row, scrow);
            if (scrow.elements.size())
            {
                sc.baseElement->rows.append(scrow);
            }
        }
    }
}

void ScreenConfigScreen::procRowSel(ScreenConfigRow *row, QVector<ScreenConfigRow *> &sel)
{
    if (row != nullptr)
    {
        if (row->selected)
        {
            sel.push_back(row);
            return;
        }

        int eleCt = row->hmsplit->count();
        for (int j = 0; j < eleCt; j++)
        {
            const QObject *w = row->hmsplit->widget(j);
            const ScreenConfigElement *ele = dynamic_cast<const ScreenConfigElement *>(w);
            if (ele)
            {
                if (ele->getIsSplitElement())
                {
                    int vCt = ele->vmsplit->count();
                    for (int i = 0; i < vCt; i++)
                    {
                        ScreenConfigRow *r = dynamic_cast<ScreenConfigRow *>(ele->vmsplit->widget(i));
                        if (r != nullptr)
                        {
                            procRowSel(r, sel);
                        }
                    }
                }
            }
        }
    }
}
QVector<ScreenConfigRow *> ScreenConfigScreen::getSelected()
{
    QVector<ScreenConfigRow *> sel;

    int vCt = vmsplit->count();
    for (int i = 0; i < vCt; i++)
    {
        QObject *w = vmsplit->widget(i);
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row)
        {
            if (row->selected)
            {
                sel.push_back(row);
            }
            else
            {
                procRowSel(row, sel);
            }
        }
    }

    return sel;
}


void ScreenConfigScreen::on_addRowButton_clicked()
{
    trace("ScreenConfig::on_addRowButton_clicked()");
    QObject *last = nullptr;
    QObject *wlast = nullptr;

    if (vmsplit->count())
    {
        last = vmsplit->widget(vmsplit->count() - 1);
        wlast = last;
    }

    baseElement->addRowAfter(dynamic_cast<ScreenConfigRow *>(wlast));
    checkAddButtons();
}
void ScreenConfigScreen::checkAddButtons()
{
    ui->addRowButton->setVisible(vmsplit->count() == 0);
    ui->addColumnLeftButton->setVisible(vmsplit->count() != 0);
    ui->addColumnRightButton->setVisible(vmsplit->count() != 0);
}
// ScreenConfigRow *ScreenConfigScreen::combineRows(ScreenConfigElement * e, int top, int bottom)
// {
//     QWidget *qli = e->vbl->itemAt(top)->widget();
//     ScreenConfigRow *scr = dynamic_cast<ScreenConfigRow *>(qli);

//     scr->parentElement->addRowBefore(scr);
//     ScreenConfigRow *newRow = dynamic_cast<ScreenConfigRow *>(scr->parentElement->vbl->itemAt(top)->widget());

//     //new row needs a split element, to which we add our old rows

//     QWidget *w = newRow->hbl->itemAt(0)->widget();
//     ScreenConfigElement *split = dynamic_cast<ScreenConfigElement *>(w);
//     split->setIsSplitElement(true);
//     split->setType(sctSplit);

//     for (int i = top + 1; i <= bottom + 1; i++) // +1 as we added a new row before top
//     {
//         // keep taking the top of the old, and put it back at the bottom of the new
//         QLayoutItem *l = e->vbl->takeAt(top + 1);

//         split->vbl->addItem(l);
//         // reset the parentage, or it all displays in the wrong place
//         l->widget()->setParent(split);
//         ScreenConfigRow *newscr = dynamic_cast<ScreenConfigRow *>(l->widget());
//         newscr->parentElement = split;
//     }
//     return  newRow;
// }
ScreenConfigRow *ScreenConfigScreen::combineRows(ScreenConfigElement * e, int top, int bottom)
{
    //combine multiple rows into rows in single element, so we can add column before/after

    QObject *qli = e->vmsplit->widget(top);
    ScreenConfigRow *scr = dynamic_cast<ScreenConfigRow *>(qli);

    scr->parentElement->addRowBefore(scr);
    ScreenConfigRow *newRow = dynamic_cast<ScreenConfigRow *>(scr->parentElement->vmsplit->widget(top));

    //new row needs a split element, to which we add our old rows

    QObject *w = newRow->hmsplit->widget(0);
    ScreenConfigElement *split = dynamic_cast<ScreenConfigElement *>(w);
    split->setIsSplitElement(true);
    split->setType(sctSplit);

    for (int i = top + 1; i <= bottom + 1; i++) // +1 as we added a new row before top
    {
        // keep taking the top of the old, and put it back at the bottom of the new
        QObject *l = e->vmsplit->widget(top+ 1);
        l->setParent(nullptr);
        QWidget *w = dynamic_cast<QWidget *>(l);
        split->vmsplit->addWidget(w);
        // reset the parentage, or it all displays in the wrong place
        //l->setParent(split);
        ScreenConfigRow *newscr = dynamic_cast<ScreenConfigRow *>(l);
        newscr->parentElement = split;
    }
    return  newRow;
}
void ScreenConfigScreen::addColumnLeft(ScreenConfigElement * e, int top, int bottom)
{
    ScreenConfigRow *newRow = combineRows(e, top, bottom);
    newRow->addLeft(nullptr);
}

void ScreenConfigScreen::addColumnRight(ScreenConfigElement * e, int top, int bottom)
{
    ScreenConfigRow *newRow = combineRows(e, top, bottom);
    newRow->addRight(nullptr);
}

int ScreenConfigScreen::getTopRow(ScreenConfigElement * e)
{

    int vCt = e->vmsplit->count();

    for (int i = 0; i < vCt; i++)
    {
        QObject *w = e->vmsplit->widget(i);
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row && row->selected)
        {
            return i;
        }
    }
    return -1;
}

int ScreenConfigScreen::getBottomRow(ScreenConfigElement * e)
{
    int vCt = e->vmsplit->count();

    for (int i = vCt - 1; i >= 0; i--)
    {
        QObject *w = e->vmsplit->widget(i);
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row && row->selected)
        {
            return i;
        }
    }
    return -1;
}

void ScreenConfigScreen::on_addColumnRightButton_clicked()
{
    QVector<ScreenConfigRow *> sel = getSelected();
    if (sel.count() <= 1)
        return ;

    ScreenConfigElement * e = sel[0]->parentElement;

    int topRow = getTopRow(e);
    int bottomRow = getBottomRow(e);

    if (topRow < 0 || topRow == bottomRow)
    {
        mShowMessage(tr("Please select (by mouse click) the top and bottom rows for the (right) column.\n"
                     "The row background will change colour when selected."), this);
    }
    else
    {
        int vCt = e->vmsplit->count();
        for (int i = 0; i < vCt; i++)
        {
            QObject *w = e->vmsplit->widget(i);
            ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
            if (row && row->selected)
            {
                row->setStyleSheet("background-color: light grey;");
                row->selected = false;
            }
        }
        addColumnRight(e, topRow, bottomRow);
    }
}

void ScreenConfigScreen::on_addColumnLeftButton_clicked()
{
    QVector<ScreenConfigRow *> sel = getSelected();
    if (sel.count() <= 1)
        return ;

    ScreenConfigElement * e = sel[0]->parentElement;

    int topRow = getTopRow(e);
    int bottomRow = getBottomRow(e);
    if (topRow < 0 || topRow == bottomRow)
    {
        mShowMessage(tr("Please select (by mouse click) the top and bottom rows for the (left) column.\n"
                     "The row background will change colour when selected."), this);
    }
    else
    {
        int vCt = e->vmsplit->count();
        for (int i = 0; i < vCt; i++)
        {
            QObject *w = e->vmsplit->widget(i);
            ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
            if (row && row->selected)
            {
                row->setStyleSheet("background-color: light grey;");
                row->selected = false;
            }
        }
        addColumnLeft(e, topRow, bottomRow);
    }
}

void ScreenConfigScreen::on_screenNameEdit_textEdited(const QString &arg1)
{
    name = arg1;
    parentElement->setScreenName(this);
}
