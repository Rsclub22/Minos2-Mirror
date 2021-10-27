#include "MShowMessageDlg.h"
#include "RSConfigure.h"
#include "ui_RSConfigure.h"

RSConfigure::RSConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RSConfigure)
{
    ui->setupUi(this);
}

RSConfigure::~RSConfigure()
{
    delete ui;
}

void RSConfigure::setServerList(QStringList rigServers, QString mainCurrent, QString subCurrent)
{
    ui->mainControl->clear();
    ui->mainControl->addItems(rigServers);
    ui->mainControl->setCurrentText(mainCurrent);

    ui->subControl->clear();
    ui->subControl->addItems(rigServers);
    ui->subControl->setCurrentText(subCurrent);
}

QString RSConfigure::getMainServer()
{
    return ui->mainControl->currentText();
}
QString RSConfigure::getSubServer()
{
    return ui->subControl->currentText();
}
void RSConfigure::on_OKButton_clicked()
{
    if (getMainServer() == getSubServer())
    {
        mShowMessage("Main and Sub rig cannot be on the same RigControl Instance", this);
    }
    else
    {
        accept();
    }
}

void RSConfigure::on_canceButton_clicked()
{
    reject();
}
