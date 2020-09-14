#include "addtransverterdialog.h"
#include "ui_addtransverterdialog.h"
#include "BandList.h"

AddTransVerterDialog::AddTransVerterDialog(QVector<QSharedPointer<BandInfo> > &_bands, QStringList& _transVertNames, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTransVerterDialog)
{
    ui->setupUi(this);
    bands = _bands;
    transVerterNames = _transVertNames;
    loadBandSel();

}



AddTransVerterDialog::~AddTransVerterDialog()
{
    delete ui;
}


void AddTransVerterDialog::loadBandSel()
{
    ui->bandSel->clear();
    for (int i = 0; i < bands.count(); i++ )
    {
        if (!checkBandUsed(bands[i]->name(), transVerterNames))
        {
            if (bands[i]->name() != "28 MHz")
            {
                ui->bandSel->addItem(bands[i]->name());
            }

        }
    }
}

QString AddTransVerterDialog::getTransVerterName()
{
    bName = ui->bandSel->currentText();
    return bName;
}


bool AddTransVerterDialog::checkBandUsed(const QString &bName, const QStringList &transVerterNames)
{

    for (int i = 0; i < transVerterNames.count(); i++)
    {
        if (bName == transVerterNames[i])
        {
            return true;
        }
    }
    return false;

}
