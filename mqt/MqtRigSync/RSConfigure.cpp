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

void RSConfigure::setServerList(QStringList rigServers, QString current)
{
    ui->subControl->clear();
    ui->subControl->addItems(rigServers);
    ui->subControl->setCurrentText(current);
}

QString RSConfigure::getSubServer()
{
    return ui->subControl->currentText();
}
void RSConfigure::on_OKButton_clicked()
{
    accept();
}

void RSConfigure::on_canceButton_clicked()
{
    reject();
}
