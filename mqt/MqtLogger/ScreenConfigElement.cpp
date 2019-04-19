#include "ScreenConfigFile.h"
#include "ScreenConfigRow.h"
#include <QStandardItemModel>

#include "ScreenConfigElement.h"
#include "ui_ScreenConfigElement.h"

static QVector <SCTypeOption> scoptions =
{
    {sctAux, "Auxiliary", "Auxiiary Display"},
    {sctChat, "Chat Display", "Chat Display"},
    {sctCluster, "Cluster Display", "Cluster Display"},
    {sctLog, "Log List", "QSO Log List"},
    {sctNextQSODetails, "Next QSO Details", "Next QSO details"},
    {sctQSOEdit, "QSO Edit", "QSO Edit"},
    {sctRigControl, "Rig Control", "Rig Control"},
    {sctRotControl, "Rotator Control", "Rotator Control"},
    {sctRotPresets, "Rotator Presets", "Rotator Presets"},
    {sctThisMatch, "This Contest Match", "This Contest Matches"},
    {sctOtherMatch, "Other Contest Match", "Other Contest Matches" },
    {sctArchiveMatch, "Archive Match", "Archive List Matches" },
    {sctWsjtx, "WSJT-X Connector", "WSJT-X Connector" },
    {sctSplit, "HSplit", "Horizontally split element"},
    {sctNone, "None", "Not in use"}
};
SCType getScreenType(QString s)
{
    foreach(const SCTypeOption &opt, scoptions)
    {
        if (opt.s == s)
            return opt.type;
    }
    return sctNone;
}
QString getScreenTypeString(SCType t)
{
    foreach(const SCTypeOption &opt, scoptions)
    {
        if (opt.type == t)
            return opt.s;
    }
    return getScreenTypeString(sctNone);

}
void ScreenConfigElement::eraseLayout(QLayout * layout)
{
    while(layout->count() > 0)
    {
        QLayoutItem *item = layout->takeAt(0);

        QWidget* widget = item->widget();
        if(widget)
        {
            delete widget;
        }
        else
        {
            QLayout * layout = item->layout();
            if (layout)
            {
                eraseLayout(layout);
            }
            else
            {
                QSpacerItem * si = item->spacerItem();
                if (si)
                {
                    delete si;
                }
            }
        }
        delete item;
    }
}
bool ScreenConfigElement::getIsSplitElement() const
{
    return isSplitElement;
}

void ScreenConfigElement::setIsSplitElement(bool value)
{
    isSplitElement = value;
    ui->splitFrame->setVisible(!value);
    ui->elementTypeCombo->setVisible(!value);
    ui->auxTypeCombo->setVisible(!value);
}

ScreenConfigElement::ScreenConfigElement(ScreenConfigRow *parentrow, ScreenConfig *sc) :
    QFrame(nullptr)
  , ui(new Ui::ScreenConfigElement)
  , parentRow(parentrow)
  , parentDialog(sc)
{
    ui->setupUi(this);

    if (parentDialog)
        vbl = parentDialog->vbl;
    else
    {
        vbl = ui->eleRowsVbl;
    }

    ui->elementTypeCombo->clear();
    int i = 0;
    int row = -1;
    foreach(const SCTypeOption &opt, scoptions)
    {
        if (opt.type == sctSplit)
        {
            row = i;
        }
        ui->elementTypeCombo->addItem(opt.s, opt.type);
        ui->elementTypeCombo->setItemData( i++, opt.hint, Qt::ToolTipRole );
    }

    QStandardItemModel *model = dynamic_cast< QStandardItemModel * >( ui->elementTypeCombo->model() );
    QStandardItem *item = model->item( row, 0 );
    item->setEnabled( false );

    i = 0;
    foreach(const AuxTypeOption &opt, auxoptions)
    {
        ui->auxTypeCombo->addItem(opt.s, opt.type);
        ui->auxTypeCombo->setItemData( i++, opt.hint, Qt::ToolTipRole );
    }
}

ScreenConfigElement::~ScreenConfigElement()
{
    delete ui;
}

void ScreenConfigElement::setType(SCType t)
{
    ui->elementTypeCombo->setCurrentText(getScreenTypeString(t));

    ui->auxTypeCombo->setVisible(t == sctAux);
}
QString ScreenConfigElement::getType() const
{
    return ui->elementTypeCombo->currentText();
}

void ScreenConfigElement::setAuxType(AuxEntries ae)
{
    ui->auxTypeCombo->setCurrentText(getAuxTypeString(ae));
}
QString ScreenConfigElement::getAuxType() const
{
    return ui->auxTypeCombo->currentText();
}

void ScreenConfigElement::on_elementTypeCombo_activated(const QString &/*arg1*/)
{
    trace("ScreenConfigElement::on_elementTypeCombo_activated");

    // if not aux, check only one of the type - make the other one "none"

    // if aux, check no more than STACKITEMS - 1 - if more, make this one "none"
    // can we disable "aux" when there are already enough?

    // https://stackoverflow.com/questions/38915001/disable-specific-items-in-qcombobox

    if (!parentRow->checkOk(this))
    {
        setType(sctNone);
    }
    SCType t = getScreenType(ui->elementTypeCombo->currentText());
    ui->auxTypeCombo->setVisible(t == sctAux);

}

void ScreenConfigElement::on_addLeftButton_clicked()
{
    trace("ScreenConfigElement::on_addLeftButton_clicked");
    parentRow->addLeft(this);
}

void ScreenConfigElement::on_addRightButton_clicked()
{
    trace("ScreenConfigElement::on_addRightButton_clicked");
    parentRow->addRight(this);
}

void ScreenConfigElement::on_removeButton_clicked()
{
    trace("ScreenConfigElement::on_removeButton_clicked");
    // how do we re-add once all elements are removed?
    if (getIsSplitElement())
    {
        if (!sShowOKCancelMessage(this, "Do you really want to delete all the rows in this element?"))
            return;
    }
    parentRow->remove(this);
}

void ScreenConfigElement::on_splitAboveButton_clicked()
{
    // Split, empty element above
    // this element becomes two ScreenConfigRow between a splitter, each with a simgle element

    // build new ScreenConfigElement

    // and replace "this" with the new one
    trace("ScreenConfigElement::on_splitAboveButton_clicked");

    QString t = getType();
    QString aux = getAuxType();
    setIsSplitElement(true);
    setType(sctSplit);

    ScreenConfigRow *baseRow = new ScreenConfigRow(this);
    vbl->insertWidget( 0, baseRow);
    baseRow->addLeft(nullptr );

    ScreenConfigRow *newRow = new ScreenConfigRow(this);
    vbl->insertWidget( 1, newRow);
    ScreenConfigElement *e = newRow->addLeft(nullptr);

    e->setType(getScreenType(t));
    e->setAuxType(getAuxEntryType(aux));
}

void ScreenConfigElement::on_splitBelowButton_clicked()
{
    trace("ScreenConfigElement::on_splitBelowButton_clicked");
    // Split, empty element below
    // this element becomes two ScreenConfigRow between a splitter, each with a simgle element

    // build new ScreenConfigElement

    // and replace "this" with the new one

    QString t = getType();
    QString aux = getAuxType();
    setIsSplitElement(true);

    ScreenConfigRow *baseRow = new ScreenConfigRow(this);
    vbl->insertWidget( 0, baseRow);
    ScreenConfigElement *e = baseRow->addLeft(nullptr );


    ScreenConfigRow *newRow = new ScreenConfigRow(this);
    vbl->insertWidget( 1, newRow);
    newRow->addLeft(nullptr);

    e->setType(getScreenType(t));
    e->setAuxType(getAuxEntryType(aux));
}
void ScreenConfigElement::addRowBefore(ScreenConfigRow *r)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == r)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigRow *baseRow = new ScreenConfigRow(this);
    vbl->insertWidget( pos, baseRow);
    baseRow->addLeft(nullptr);
    if (parentDialog)
        parentDialog->checkAddRowButton();
}
void ScreenConfigElement::removeRow(ScreenConfigRow *r)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == r)
        {
            pos = i;
            break;
        }
    }
    QLayoutItem *taken = vbl->takeAt(pos);
    if (taken)
    {
        // From the source, I don't think the deleting the layout item deletes the widget
        taken->widget()->deleteLater();
        delete taken;
    }
    if (parentDialog)
    {
        parentDialog->checkAddRowButton();
    }
    else if (getIsSplitElement())
    {
        if (vbl->count() == 0)
        {
            parentRow->remove(this);
        }
    }


}
void ScreenConfigElement::addRowAfter(ScreenConfigRow *r)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == r)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigRow *baseRow = new ScreenConfigRow(this);
    vbl->insertWidget( pos + 1, baseRow);
    baseRow->addLeft(nullptr);
    if (parentDialog)
        parentDialog->checkAddRowButton();
}
bool ScreenConfigElement::checkOk(ScreenConfigElement *e)
{
    if (parentDialog)
        return  parentDialog->checkOk(e);

    if (parentRow)
        return  parentRow->checkOk(e);

    return true;
}
