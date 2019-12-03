#include "kstconfigure.h"
#include "ui_kstconfigure.h"

KSTConfigure::KSTConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KSTConfigure)
{
    ui->setupUi(this);
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

    if (hostname.isEmpty() || port.isEmpty() ||username.isEmpty() ||password.isEmpty() || locator.isEmpty())
        return;

    accept();
}
