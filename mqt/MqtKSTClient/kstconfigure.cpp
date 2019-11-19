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

    QStringList services =
    {"50/70 MHz..............1",
    "144/432 MHz............2",
    "Microwave..............3",
    "EME/JT65...............4",
    "Low Band...............5",
    "50 MHz IARU Region 3...6",
    "50 MHz IARU Region 2...7",
    "144/432 MHz IARU R 2...8",
    "144/432 MHz IARU R 3...9",
    "kHz (2000-630m).......10",
    "Warc (30,17,12m)......11"};

    ui->serviceCombo->addItems(services);
    int s = service.toInt();
    ui->serviceCombo->setCurrentIndex(s - 1);

    ui->serverEdit->setText(hostname);
    ui->portEdit->setText(port);
    ui->callsignEdit->setText(username);
    ui->passwordEdit->setText(password);

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
    int s = ui->serviceCombo->currentIndex() + 1;
    service = QString::number(s);

    hostname = ui->serverEdit->text();
    port = ui->portEdit->text();
    username = ui->callsignEdit->text();
    password = ui->passwordEdit->text();

    if (hostname.isEmpty() || port.isEmpty() ||username.isEmpty() ||password.isEmpty())
        return;

    accept();
}
