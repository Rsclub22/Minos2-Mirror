#include "kstconfigure.h"
#include "ui_kstconfigure.h"


KSTConfigure::KSTConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KSTConfigure)
{
    ui->setupUi(this);
    ui->portFrame->setVisible(false);
    ui->serverFrame->setVisible(false);

    ui->callsignEdit->setValidator(&userValidator);
    ui->locatorEdit->setValidator(&locValidator);

}
int KSTConfigure::exec()
{
    ui->portEdit->setValidator(new QIntValidator(0, 0xffff, this));

    ui->serverEdit->setText(hostname);
    ui->portEdit->setText(port);
    ui->callsignEdit->setText(username);
    ui->passwordEdit->setText(password);

    ui->locatorEdit->setText(locator);
    ui->autoConnect->setChecked(autoConnect);

    ui->ASServerName->setText(ASServerName);
    ui->ASMyName->setText(ASMyName);

    ui->ASMaxDistance->setText(QString::number(ASMaxDistance));
    ui->ASMaxDistance->setValidator(new QIntValidator(0, 0xffff, this));
    ui->ASMinDistance->setText(QString::number(ASMinDistance));
    ui->ASMinDistance->setValidator(new QIntValidator(0, 0xffff, this));
    ui->ASPort->setText(QString::number( ASPort));
    ui->ASPort->setValidator(new QIntValidator(0, 0xffff, this));
    ui->ASTimeout->setText(QString::number( ASTimeout));
    ui->ASTimeout->setValidator(new QIntValidator(1, 60, this));

    return QDialog::exec();
}
KSTConfigure::~KSTConfigure()
{
    delete ui;
}

void KSTConfigure::on_cancelButton_clicked()
{
    reject();
}

void KSTConfigure::on_OKButton_clicked()
{
    hostname = ui->serverEdit->text();
    port = ui->portEdit->text();
    username = ui->callsignEdit->text();
    password = ui->passwordEdit->text();
    autoConnect = ui->autoConnect->isChecked();
    locator = ui->locatorEdit->text().toUpper();

    ASServerName = ui->ASServerName->text();
    ASMyName = ui->ASMyName->text();

    ASMaxDistance = ui->ASMaxDistance->text().toInt();
    ASMinDistance = ui->ASMinDistance->text().toInt();

    ASPort = ui->ASPort->text().toInt();
    ASTimeout = ui->ASTimeout->text().toInt();

    if (hostname.isEmpty() || port.isEmpty() ||username.isEmpty() ||password.isEmpty() || locator.isEmpty())
        return;



    accept();
}
