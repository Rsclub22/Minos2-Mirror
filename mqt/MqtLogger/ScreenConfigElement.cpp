#include "ScreenConfigFile.h"
#include "ScreenConfigRow.h"
#include <QStandardItemModel>
#include <QListView>

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
    {sctRunButtons, QT_TR_NOOP("Call Freq Buttons"), QT_TR_NOOP("Call Freq Buttons")},
    {sctRotControl, QT_TR_NOOP("Rotator Control"), QT_TR_NOOP("Rotator Control")},
    {sctRotPresets, QT_TR_NOOP("Rotator Presets"), QT_TR_NOOP("Rotator Presets")},
    {sctThisMatch, QT_TR_NOOP("This Contest Match"), QT_TR_NOOP("This Contest Matches")},
    {sctOtherMatch, QT_TR_NOOP("Other Contest Match"), QT_TR_NOOP("Other Contest Matches") },
    {sctArchiveMatch, QT_TR_NOOP("Archive Match"), QT_TR_NOOP("Archive List Matches") },
    {sctWsjtx, QT_TR_NOOP("WSJT-X Connector"), QT_TR_NOOP("WSJT-X Connector") },
    {sctBandmap, QT_TR_NOOP("Bandmap Display"), QT_TR_NOOP("Bandmap Display")},
    {sctSplit, QT_TR_NOOP("HSplit"), QT_TR_NOOP("Horizontally split element")},
    {sctNone, QT_TR_NOOP("None"), QT_TR_NOOP("Not in use")}
};
SCType ScreenConfigElement::getScreenType(QString s)
{
    for(auto const  &opt: scoptions)
    {
        if ((opt.s == s) || (tr(opt.s) == s))
            return opt.type;
    }
    return sctNone;
}
const char * ScreenConfigElement::getRawScreenTypeString(SCType t)
{
    for(auto const  &opt: scoptions)
    {
        if (opt.type == t)
            return opt.s;
    }
    return getRawScreenTypeString(sctNone);

}
QString ScreenConfigElement::getTrScreenTypeString(SCType t)
{
    return tr(getRawScreenTypeString(t));
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
    for(auto const  &opt: scoptions)
    {
        if (opt.type == sctMainScreen || opt.type == sctScreen)
        {
            continue;   // don't allow these to be selected
        }
        if (opt.type == sctSplit)
        {
            row = i;
        }
        ui->elementTypeCombo->addItem(tr(opt.s), opt.type);
        ui->elementTypeCombo->setItemData( i++, tr(opt.hint), Qt::ToolTipRole );
    }

    QStandardItemModel *model = dynamic_cast< QStandardItemModel * >( ui->elementTypeCombo->model() );
    QStandardItem *item = model->item( row, 0 );
    item->setEnabled( false );
    qobject_cast<QListView *>(ui->elementTypeCombo->view())->setRowHidden(row, true);

    i = 0;
    for(auto const &opt: StackedInfoFrame::auxoptions)
    {
        QString s = StackedInfoFrame::getTrAuxTypeString(opt.type);
        ui->auxTypeCombo->addItem(s, opt.type);
        ui->auxTypeCombo->setItemData( i++, tr(opt.hint), Qt::ToolTipRole );
    }
}

ScreenConfigElement::~ScreenConfigElement()
{
    delete ui;
}

void ScreenConfigElement::setType(SCType t)
{
    ui->elementTypeCombo->setCurrentText(getTrScreenTypeString(t));

    ui->auxTypeCombo->setVisible(t == sctAux);
}
QString ScreenConfigElement::getType() const
{
    return ui->elementTypeCombo->currentText();
}

void ScreenConfigElement::setAuxType(AuxEntries ae)
{
    ui->auxTypeCombo->setCurrentText(StackedInfoFrame::getTrAuxTypeString(ae));
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

    screenConfigDialog->checkAddButtons();
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
    e->setAuxType(StackedInfoFrame::getAuxEntryType(aux));
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
    e->setAuxType(StackedInfoFrame::getAuxEntryType(aux));
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

    screenConfigDialog->checkAddButtons();

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

    screenConfigDialog->checkAddButtons();

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

    screenConfigDialog->checkAddButtons();
}
bool ScreenConfigElement::checkOk(ScreenConfigElement *e)
{
    if (parentDialog)
        return  parentDialog->checkOk(e);

    if (parentRow)
        return  parentRow->checkOk(e);

    return true;
}
