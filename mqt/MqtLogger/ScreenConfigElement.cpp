#include <QStandardItemModel>
#include <QListView>

#include "QtUtils.h"
#include "MMessageDialog.h"
#include "ScreenConfigFile.h"
#include "ScreenConfigScreen.h"
#include "ScreenConfigRow.h"
#include "MTrace.h"
#include "cutils.h"
#include "ScreenConfigElement.h"
#include "ui_ScreenConfigElement.h"

QVector <SCTypeOption> ScreenConfigElement::scoptions =
{
    {sctMainScreen, QT_TR_NOOP("mainscreen"), QT_TR_NOOP("Main Screen")},
    {sctScreen, QT_TR_NOOP("screen"), QT_TR_NOOP("Secondary Screen")},
    {sctAux, QT_TR_NOOP("Auxiliary"), QT_TR_NOOP("Auxiliary Display")},
    {sctChat, QT_TR_NOOP("Chat Display"), QT_TR_NOOP("Chat Display")},
    {sctCluster, QT_TR_NOOP("Cluster Display"), QT_TR_NOOP("Cluster Display")},
    {sctLog, QT_TR_NOOP("Log List"), QT_TR_NOOP("QSO Log List")},
    {sctNextQSODetails, QT_TR_NOOP("Next QSO Details"), QT_TR_NOOP("Next QSO details")},
    {sctQSOEdit, QT_TR_NOOP("QSO Edit"), QT_TR_NOOP("QSO Edit")},
    {sctRigControl, QT_TR_NOOP("Rig Control"), QT_TR_NOOP("Rig Control")},
    {sctBandSwitch, QT_TR_NOOP("HF Band Switching"), QT_TR_NOOP("HF Band Switching")},
    {sctRunButtons, QT_TR_NOOP("Call Freq Buttons"), QT_TR_NOOP("Run Freq Buttons")},
    {sctRotControl, QT_TR_NOOP("Rotator Control"), QT_TR_NOOP("Rotator Control")},
    {sctSkyScanControl, QT_TR_NOOP("SkyScan Control"), QT_TR_NOOP("SkyScan Control")},
    {sctRotCompassDisplay, QT_TR_NOOP("Rotator Compass Display"), QT_TR_NOOP("Rotator Compass Display")},
    {sctRotPresets, QT_TR_NOOP("Rotator Presets"), QT_TR_NOOP("Rotator Presets")},
    {sctThisMatch, QT_TR_NOOP("This Contest Match"), QT_TR_NOOP("This Contest Matches")},
    {sctOtherMatch, QT_TR_NOOP("Other Contest Match"), QT_TR_NOOP("Other Contest Matches") },
    {sctArchiveMatch, QT_TR_NOOP("Archive Match"), QT_TR_NOOP("Archive List Matches") },
    {sctWsjtx, QT_TR_NOOP("WSJT-X Connector"), QT_TR_NOOP("WSJT-X Connector") },
    {sctBandmap, QT_TR_NOOP("Bandmap Display"), QT_TR_NOOP("Bandmap Display")},
    {sctSplit, QT_TR_NOOP("HSplit"), QT_TR_NOOP("Horizontally split element")},
    {sctTxVmButtons, QT_TR_NOOP("Keyer"), QT_TR_NOOP("Keyer")},
    {sctQrzDisplay, QT_TR_NOOP("QRZ Display"), QT_TR_NOOP("QRZ Display")},
    {sctQsoMap, QT_TR_NOOP("QSO Map"), QT_TR_NOOP("QSO Map")},
    {sctDMButtons, QT_TR_NOOP("Data Modes Buttons"), QT_TR_NOOP("Data Modes Buttons")},
    {sctNone, QT_TR_NOOP("None"), QT_TR_NOOP("Not in use")}
};
SCType ScreenConfigElement::getScreenType(QString s)
{
    for(auto const  &opt: QASCONST(scoptions))
    {
        if ((opt.s == s) || (tr(opt.s) == s))
            return opt.type;
    }
    return sctNone;
}
const char * ScreenConfigElement::getRawScreenTypeString(SCType t)
{
    for(auto const  &opt: QASCONST(scoptions))
    {
        if (opt.type == t)
            return opt.s;
    }
    return getRawScreenTypeString(sctNone);

}
const char * ScreenConfigElement::getRawScreenHint(SCType t)
{
    for(auto const  &opt: QASCONST(scoptions))
    {
        if (opt.type == t)
            return opt.hint;
    }
    return getRawScreenHint(sctNone);

}
QString ScreenConfigElement::getTrScreenHint(SCType t)
{
    return tr(getRawScreenHint(t));
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

ScreenConfigElement::ScreenConfigElement(ScreenConfigRow *parentrow, ScreenConfigScreen *sc) :
    QFrame(nullptr)
  , ui(new Ui::ScreenConfigElement)
  , parentRow(parentrow)
  , parentDialog(sc)
{
    ui->setupUi(this);

    // colours from https://www.december.com/html/spec/colorsvg.html
    setStyleSheet("#ScreenConfigElement { border: 2px solid darkmagenta; }");

    if (parentDialog)
        vbl = parentDialog->vbl;
    else
    {
        vbl = ui->eleRowsVbl;
    }

    ui->elementTypeCombo->clear();
    int i = 0;
    int row = -1;
    for(auto const  &opt: QASCONST(scoptions))
    {
        if (opt.type == sctMainScreen || opt.type == sctScreen)
        {
            continue;   // don't allow these to be selected
        }
        if (opt.type == sctSplit)
        {
            row = i;
        }
        const char *disp = opt.hint;
        ui->elementTypeCombo->addItem(tr(disp), opt.type);
        ui->elementTypeCombo->setItemData( i++, tr(opt.hint), Qt::ToolTipRole );
    }

    // Find the "horizontal split" element
    QStandardItemModel *model = dynamic_cast< QStandardItemModel * >( ui->elementTypeCombo->model() );
    QStandardItem *item = model->item( row, 0 );
    item->setEnabled( false );
    QModelIndex ind = model->index(row, 0);

    // for sorting you need the following 4 lines
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(ui->elementTypeCombo);
    proxy->setSourceModel(model);
    // combo's current model must be reparented,
    // otherwise QComboBox::setModel() will delete it
    model->setParent(proxy);
    ui->elementTypeCombo->setModel(proxy);
    // sort
    ui->elementTypeCombo->model()->sort(0); // Column 0

    // and disable h_split in the PROXY position
    QModelIndex pind = proxy->mapFromSource(ind);
    qobject_cast<QListView *>(ui->elementTypeCombo->view())->setRowHidden(pind.row(), true);

    i = 0;
    for(auto const &opt: QASCONST(StackedInfoFrame::auxoptions))
    {
        QString s = StackedInfoFrame::getTrAuxTypeString(opt.type);
        ui->auxTypeCombo->addItem(s, opt.type);
        ui->auxTypeCombo->setItemData( i++, tr(opt.hint), Qt::ToolTipRole );
    }
    // for sorting you need the following 4 lines
    proxy = new QSortFilterProxyModel(ui->auxTypeCombo);
    proxy->setSourceModel(ui->auxTypeCombo->model());
    // combo's current model must be reparented,
    // otherwise QComboBox::setModel() will delete it
    ui->auxTypeCombo->model()->setParent(proxy);
    ui->auxTypeCombo->setModel(proxy);
    // sort
    ui->auxTypeCombo->model()->sort(0); // Column 0
}

ScreenConfigElement::~ScreenConfigElement()
{
    delete ui;
}

void ScreenConfigElement::setType(SCType t)
{
    QString s = getTrScreenHint(t);
    ui->elementTypeCombo->setCurrentText(s);

    ui->auxTypeCombo->setVisible(t == sctAux);
}
SCType ScreenConfigElement::getType() const
{
    int t = ui->elementTypeCombo->itemData(ui->elementTypeCombo->currentIndex()).toInt();
    return static_cast<SCType>(t);
}

void ScreenConfigElement::setAuxType(AuxEntries ae)
{
    ui->auxTypeCombo->setCurrentText(StackedInfoFrame::getTrAuxTypeString(ae));
}
AuxEntries ScreenConfigElement::getAuxType() const
{
    int t = ui->auxTypeCombo->itemData(ui->auxTypeCombo->currentIndex()).toInt();
    return static_cast<AuxEntries>(t);
}

void ScreenConfigElement::on_elementTypeCombo_activated(int /*arg1*/)
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
    SCType t = getType();
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

    screenConfigDialog->curScreen->checkAddButtons();
}

void ScreenConfigElement::on_splitAboveButton_clicked()
{
    // Split, empty element above
    // this element becomes two ScreenConfigRow between a splitter, each with a simgle element

    // build new ScreenConfigElement

    // and replace "this" with the new one
    trace("ScreenConfigElement::on_splitAboveButton_clicked");

    SCType t = getType();
    AuxEntries aux = getAuxType();
    setIsSplitElement(true);
    setType(sctSplit);

    ScreenConfigRow *baseRow = new ScreenConfigRow(this);
    vbl->insertWidget( 0, baseRow);
    baseRow->addLeft(nullptr );

    ScreenConfigRow *newRow = new ScreenConfigRow(this);
    vbl->insertWidget( 1, newRow);
    ScreenConfigElement *e = newRow->addLeft(nullptr);

    e->setType(t);
    e->setAuxType(aux);
}

void ScreenConfigElement::on_splitBelowButton_clicked()
{
    trace("ScreenConfigElement::on_splitBelowButton_clicked");
    // Split, empty element below
    // this element becomes two ScreenConfigRow between a splitter, each with a simgle element

    // build new ScreenConfigElement

    // and replace "this" with the new one

    SCType t = getType();
    AuxEntries aux = getAuxType();
    setIsSplitElement(true);

    ScreenConfigRow *baseRow = new ScreenConfigRow(this);
    vbl->insertWidget( 0, baseRow);
    ScreenConfigElement *e = baseRow->addLeft(nullptr );


    ScreenConfigRow *newRow = new ScreenConfigRow(this);
    vbl->insertWidget( 1, newRow);
    newRow->addLeft(nullptr);

    e->setType(t);
    e->setAuxType(aux);
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

    screenConfigDialog->curScreen->checkAddButtons();

}
void ScreenConfigElement::removeRow(ScreenConfigRow *r)
{
    int pos = 0;
    int vct = vbl->count();
    for (int i = 0; i < vct; i++)
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

    else if (getIsSplitElement())
    {
        if (vbl->count() == 0)
        {
            parentRow->remove(this);
        }
    }

    screenConfigDialog->curScreen->checkAddButtons();

}
void ScreenConfigElement::addRowAfter(ScreenConfigRow *r)
{
    int pos = 0;
    int offset = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == r)
        {
            pos = i;
            offset = 1;
            break;
        }
    }
    ScreenConfigRow *baseRow = new ScreenConfigRow(this);
    vbl->insertWidget( pos + offset, baseRow);
    baseRow->addLeft(nullptr);

    screenConfigDialog->curScreen->checkAddButtons();
}
bool ScreenConfigElement::checkOk(ScreenConfigElement *e)
{
    if (parentDialog)
        return  parentDialog->checkOk(e);

    if (parentRow)
        return  parentRow->checkOk(e);

    return true;
}
