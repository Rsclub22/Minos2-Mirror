#include "ScreenConfigFile.h"
#include "ScreenConfigRow.h"
#include "ScreenConfigElement.h"
#include "ui_ScreenConfigElement.h"

class SCTypeOption
{
public:
    SCType type;
    QString s;
    QString hint;
};
static QVector <SCTypeOption> options =
{
    {sctNone, "None", "Not in use"},
    {sctAux, "Aux", "Auxiiary Display"},
    {sctLog, "Log", "QSO Log List"},
    {sctRigControl, "Rig", "Rig Control"},
    {sctRotControl, "Rot", "Rotator Contro;"},
    {sctQSOEdit, "QSO", "QSO Edit"},
    {sctNextQSODetails, "Crib", "Next QSO details"},
    {sctThisMatch, "This", "This Contest Matches"},
    {sctOtherMatch, "Other", "Other Contest Matches" },
    {sctArchiveMatch, "Arch", "Archive List Matches" },
    {sctChat, "Chat", "Chat Display"}
};

ScreenConfigElement::ScreenConfigElement(QWidget *parent, ScreenConfigRow *parentrow) :
    QFrame(parent)
  , ui(new Ui::ScreenConfigElement)
  , parentRow(parentrow)
{
    ui->setupUi(this);

    int i = 0;
    foreach(const SCTypeOption &opt, options)
    {
        ui->elementTypeCombo->addItem(opt.s, opt.type);
        ui->elementTypeCombo->setItemData( i++, opt.hint, Qt::ToolTipRole );
    }
}

ScreenConfigElement::~ScreenConfigElement()
{
    delete ui;
}
void ScreenConfigElement::setType(QString t)
{
    ui->elementTypeCombo->setCurrentText(t);
}
QString ScreenConfigElement::getType()
{
    return ui->elementTypeCombo->currentText();
}
void ScreenConfigElement::on_elementTypeCombo_activated(const QString &arg1)
{

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
