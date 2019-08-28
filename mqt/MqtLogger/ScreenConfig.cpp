#include "base_pch.h"
#include <QScrollBar>

#include "ContestApp.h"
#include "tlogcontainer.h"

#include "ScreenConfigFile.h"
#include "ScreenConfigElement.h"
#include "ScreenConfigRow.h"
#include "ScreenConfigElement.h"

#include "ScreenConfig.h"
#include "ui_ScreenConfig.h"

void ScreenConfig::buildRows(QVector<SCRow> rows, ScreenConfigElement *bele, QVBoxLayout *vbl)
{
    for (int j = 0; j < rows.count(); j++)
    {
        //QWidget *w = parentWidget();
        ScreenConfigRow *baseRow = new ScreenConfigRow(bele);
        vbl->insertWidget( j, baseRow);
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
               QVBoxLayout *v = e->vbl;
               buildRows(rows[j].elements[k].rows, e, v);
           }
           baseRow->hbl->insertWidget(k, e);
       }
    }
}

ScreenConfig::ScreenConfig(QWidget *parent, ScreenConfigFile &scfp, QString curConfigNamep) :
    QDialog(parent),
    ui(new Ui::ScreenConfig),
    scf(scfp),
    curConfigName(curConfigNamep)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("ScreenConfig/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    vbl = new QVBoxLayout(ui->scrollAreaWidgetContents);
    vbl->setMargin(1);
    ui->scrollAreaWidgetContents->setLayout(vbl);

    baseElement = new ScreenConfigElement(nullptr, this);

    // create the screen contents based on the config

    SC sc = scf.configs[curConfigName];

    if (sc.baseElement == nullptr || sc.baseElement->rows.count() == 0)
    {
        on_addRowButton_clicked();
    }
    else
    {
        buildRows(sc.baseElement->rows, baseElement, vbl);

    }
    checkAddRowButton();
}

ScreenConfig::~ScreenConfig()
{
    delete ui;
}

int ScreenConfig::topRowCount()
{
    int vCt = vbl->count();
    return vCt;
}
bool ScreenConfig::isTopLevelRow(ScreenConfigRow *scr)
{
    int vCt = vbl->count();
    for (int i = 0; i < vCt; i++)
    {
        QWidget *w = vbl->itemAt(i)->widget();
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row == scr)
        {
            return true;
        }
    }
    return false;
}
void ScreenConfig::doCloseEvent()
{
    QSettings settings;
    settings.setValue("ScreenConfig/geometry", saveGeometry());
}
void ScreenConfig::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void ScreenConfig::accept()
{
    doCloseEvent();
    QDialog::accept();
}
void ScreenConfig::procRow(ScreenConfigRow *row, SCRow &scrow)
{
    if (row != nullptr)
    {
        int eleCt = row->hbl->count();
        for (int j = 0; j < eleCt; j++)
        {
            QWidget *w = row->hbl->itemAt(j)->widget();
            ScreenConfigElement *ele = dynamic_cast<ScreenConfigElement *>(w);
            if (ele)
            {
                SCElement scele;
                if (ele->getIsSplitElement())
                {
                    scele.type = sctSplit;

                    int vCt = ele->vbl->count();
                    for (int i = 0; i < vCt; i++)
                    {
                        ScreenConfigRow *r = dynamic_cast<ScreenConfigRow *>(ele->vbl->itemAt(i)->widget());
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
                    scele.type = getScreenType(ele->getType());
                    scele.auxType = getAuxEntryType(ele->getAuxType());
                }
                scrow.elements.append(scele);
            }
        }
    }
}
SC ScreenConfig::getConfig()
{
    SC sc;
    sc.name = curConfigName;
    sc.baseElement = QSharedPointer<SCElement>(new SCElement);
    sc.baseElement->type = sctSplit;

    int vCt = vbl->count();
    for (int i = 0; i < vCt; i++)
    {
        QWidget *w = vbl->itemAt(i)->widget();
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row)
        {
            SCRow scrow;
            procRow(row, scrow);
            sc.baseElement->rows.append(scrow);
        }
    }
    return sc;
}
void ScreenConfig::on_OKButton_clicked()
{
    trace("ScreenConfig::on_OKButton_clicked()");

    // analyse and apply the new layout
    SC sc = getConfig();

    // replace it in the config map
    scf.configs[curConfigName] = sc;

    close();
}

void ScreenConfig::on_applyButton_clicked()
{
    // First, analyse the screen layout into a config object
    trace("ScreenConfig::on_applyButton_clicked()");

    SC sc = getConfig();

    // replace it in the config map
    scf.configs[curConfigName] = sc;


    // write it back, or the screen redraw doesn't work
    scf.dumpFile();

    LogContainer->selectLayout(curConfigName);
    LogContainer->applyScreenLayouts();
}

void ScreenConfig::on_cancelButton_clicked()
{
    trace("ScreenConfig::on_cancelButton_clicked()");
    close();
}

bool ScreenConfig::checkRowOk(const ScreenConfigRow *row, ScreenConfigElement *e, int &auxCount)
{
    QString etype = e->getType();
    if (row)
    {
        for (int j = 0; j < row->hbl->count(); j++)
        {
            const QWidget *w = row->hbl->itemAt(j)->widget();
            const ScreenConfigElement *ele = dynamic_cast<const ScreenConfigElement *>(w);
            if (ele && ele != e)
            {
                QString type = ele->getType();
                if (ele->getIsSplitElement())
                {
                    for (int i = 0; i < ele->vbl->count(); i++)
                    {
                        ScreenConfigRow *r = dynamic_cast<ScreenConfigRow *>(ele->vbl->itemAt(i)->widget());
                        if (!checkRowOk(r, e, auxCount))
                            return false;
                    }
                }
                else if (type == etype)
                {
                    if (type == getScreenTypeString(sctAux))
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
bool ScreenConfig::checkOk(ScreenConfigElement *e)
{
    int auxCount = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        const QWidget *w = vbl->itemAt(i)->widget();
        const ScreenConfigRow *row = dynamic_cast<const ScreenConfigRow *>(w);
        if (!checkRowOk(row, e, auxCount))
            return false;

    }
    QString etype = e->getType();
    if (etype != getScreenTypeString(sctAux) || auxCount < STACKITEMS)
    {
        return true;
    }
    else if (etype != getScreenTypeString(sctCluster))
    {
        return true;
    }
    return false;
}

void ScreenConfig::on_addRowButton_clicked()
{
    trace("ScreenConfig::on_addRowButton_clicked()");
    QLayoutItem *last = nullptr;
    QWidget *wlast = nullptr;

    if (vbl->count())
    {
        last = vbl->itemAt(vbl->count() - 1);
        wlast = last->widget();
    }

    baseElement->addRowAfter(dynamic_cast<ScreenConfigRow *>(wlast));
    checkAddRowButton();
}
void ScreenConfig::checkAddRowButton()
{
    ui->addRowButton->setVisible(vbl->count() == 0);
}

ScreenConfigRow *ScreenConfig::combineRows(int top, int bottom)
{
    QWidget *qli = vbl->itemAt(top)->widget();
    ScreenConfigRow *scr = dynamic_cast<ScreenConfigRow *>(qli);

    scr->parentElement->addRowBefore(scr);
    ScreenConfigRow *newRow = dynamic_cast<ScreenConfigRow *>(scr->parentElement->vbl->itemAt(top)->widget());

    //new row needs a split element, to which we add our old rows

    QWidget *w = newRow->hbl->itemAt(0)->widget();
    ScreenConfigElement *split = dynamic_cast<ScreenConfigElement *>(w);
    split->setIsSplitElement(true);
    split->setType(sctSplit);

    for (int i = top + 1; i <= bottom + 1; i++)
    {
        // keep taking the top of the old, and put it back at the bottom of the new
        QLayoutItem *l = scr->parentElement->vbl->takeAt(top + 1);

        split->vbl->addItem(l);
        // reset the parentage, or it all displays in the wrong place
        l->widget()->setParent(split);
    }
    return  newRow;
}
void ScreenConfig::addColumnLeft(int top, int bottom)
{
    ScreenConfigRow *newRow = combineRows(top, bottom);
    newRow->addLeft(nullptr);
}

void ScreenConfig::addColumnRight(int top, int bottom)
{
    ScreenConfigRow *newRow = combineRows(top, bottom);
    newRow->addRight(nullptr);
}

int ScreenConfig::getTopRow()
{
    int vCt = vbl->count();
    for (int i = 0; i < vCt; i++)
    {
        QWidget *w = vbl->itemAt(i)->widget();
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row && row->selected)
        {
            return i;
        }
    }
    return -1;
}

int ScreenConfig::getBottomRow()
{
    int vCt = vbl->count();
    for (int i = vCt - 1; i >= 0; i--)
    {
        QWidget *w = vbl->itemAt(i)->widget();
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row && row->selected)
        {
            return i;
        }
    }
    return -1;
}
void ScreenConfig::on_addColumnRightButton_clicked()
{
    int topRow = getTopRow();
    int bottomRow = getBottomRow();

    if (topRow < 0 || topRow == bottomRow)
    {
        mShowMessage("Please select (by mouse click) the top and bottom rows for the (right) column.\r\n"
                     "The row background will change colour when selected.", this);
    }
    else
    {
        int vCt = vbl->count();
        for (int i = 0; i < vCt; i++)
        {
            QWidget *w = vbl->itemAt(i)->widget();
            ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
            if (row && row->selected)
            {
                row->setStyleSheet("background-color: light grey;");
                row->selected = false;
            }
        }
        addColumnRight(topRow, bottomRow);
    }
}

void ScreenConfig::on_addColumnLeftButton_clicked()
{
    int topRow = getTopRow();
    int bottomRow = getBottomRow();
    if (topRow < 0 || topRow == bottomRow)
    {
        mShowMessage("Please select (by mouse click) the top and bottom rows for the (left) column.\r\n"
                     "The row background will change colour when selected.", this);
    }
    else
    {
        int vCt = vbl->count();
        for (int i = 0; i < vCt; i++)
        {
            QWidget *w = vbl->itemAt(i)->widget();
            ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
            if (row && row->selected)
            {
                row->setStyleSheet("background-color: light grey;");
                row->selected = false;
            }
        }
        addColumnLeft(topRow, bottomRow);
    }
}
