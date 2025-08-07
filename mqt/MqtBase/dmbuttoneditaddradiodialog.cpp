#include "dmbuttoneditaddradiodialog.h"
#include "ui_dmbuttoneditaddradiodialog.h"

#include "cutils.h"

DmButtonEditAddRadioDialog::DmButtonEditAddRadioDialog(const QStringList listOfRadios, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DmButtonEditAddRadioDialog)

{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Add Radio"));

    comboSetUniqueNames(listOfRadios, ui->radioListCBox);

    connect (ui->radioListCBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DmButtonEditAddRadioDialog::radioNameSelect);
}

DmButtonEditAddRadioDialog::~DmButtonEditAddRadioDialog()
{
    delete ui;
}




void DmButtonEditAddRadioDialog::radioNameSelect()
{
    radioName = ui->radioListCBox->currentText();
}

QString DmButtonEditAddRadioDialog::getRadioName()
{
    return radioName.trimmed();
}
