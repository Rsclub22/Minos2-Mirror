#include "ScreenConfigFile.h"
#include "ScreenConfigRow.h"
#include "ScreenConfigElement.h"
#include "ui_ScreenConfigElement.h"

static QVector <SCTypeOption> scoptions =
{
    {sctAux, "Auxiliary", "Auxiiary Display"},
    {sctChat, "Chat Display", "Chat Display"},
    {sctLog, "Log List", "QSO Log List"},
    {sctNextQSODetails, "Next QSO Details", "Next QSO details"},
    {sctQSOEdit, "QSO Edit", "QSO Edit"},
    {sctRigControl, "Rig Control", "Rig Control"},
    {sctRotControl, "Rotator Control", "Rotator Control"},
    {sctRotPresets, "Rotator Presets", "Rotator Presets"},
    {sctThisMatch, "This Contest Match", "This Contest Matches"},
    {sctOtherMatch, "Other Contest Match", "Other Contest Matches" },
    {sctArchiveMatch, "Archive Match", "Archive List Matches" },
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
ScreenConfigElement::ScreenConfigElement(QWidget *parent, ScreenConfigRow *parentrow) :
    QFrame(parent)
  , ui(new Ui::ScreenConfigElement)
  , parentRow(parentrow)
{
    ui->setupUi(this);

    ui->elementTypeCombo->clear();
    int i = 0;
    foreach(const SCTypeOption &opt, scoptions)
    {
        ui->elementTypeCombo->addItem(opt.s, opt.type);
        ui->elementTypeCombo->setItemData( i++, opt.hint, Qt::ToolTipRole );
    }
}

ScreenConfigElement::~ScreenConfigElement()
{
    delete ui;
}
void ScreenConfigElement::setType(SCType t)
{
    ui->elementTypeCombo->setCurrentText(getScreenTypeString(t));
}
QString ScreenConfigElement::getType() const
{
    return ui->elementTypeCombo->currentText();
}
void ScreenConfigElement::on_elementTypeCombo_activated(const QString &/*arg1*/)
{
    // if not aux, check only one of the type - make the other one "none"

    // if aux, check no more than STACKITEMS - 1 - if more, make this one "none"
    // can we disable "aux" when there are already enough?

    // https://stackoverflow.com/questions/38915001/disable-specific-items-in-qcombobox

    if (!parentRow->checkOk(this))
    {
        setType(sctNone);
//        repaint();
//        QString t = getType();
//        mShowMessage(t, this);
    }
}

void ScreenConfigElement::on_addLeftButton_clicked()
{
    parentRow->addLeft(this);
}

void ScreenConfigElement::on_addRightButton_clicked()
{
    parentRow->addRight(this);
}

void ScreenConfigElement::on_removeButton_clicked()
{
    // how do we re-add once all elements are removed?
    parentRow->remove(this);
}
