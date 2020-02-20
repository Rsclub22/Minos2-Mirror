#include "kstconfigure.h"
#include "ui_kstconfigure.h"

const char *KSTConfigure::ASBandStrings[] = {
    QT_TR_NOOP("144MHz"),
    QT_TR_NOOP("432MHz"),
    QT_TR_NOOP("1.2GHz"),
    QT_TR_NOOP("2.3GHz"),
    QT_TR_NOOP("3.4GHz"),
    QT_TR_NOOP("5.7GHz"),
    QT_TR_NOOP("10GHz"),
    nullptr
};
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
    ui->maxDistanceEdit->setText(QString::number(maxDistance));
    ui->maxDistanceEdit->setValidator(new QIntValidator(0, 0xffff, this));

    ui->activateAirScoutcb->setChecked(ASActive);

    for(int i = 0; i < asbMaxBand; i++)
    {
        ui->ASFreqCombo->addItem(tr(ASBandStrings[i]));
    }

    ui->ASFreqCombo->setCurrentIndex(ASActiveBand);

    ui->ASServerName->setText(ASServerName);
    ui->ASMyName->setText(ASMyName);

    ui->ASMaxDistance->setText(QString::number(ASMaxDistance));
    ui->ASMaxDistance->setValidator(new QIntValidator(0, 0xffff, this));
    ui->ASMinDistance->setText(QString::number(ASMinDistance));
    ui->ASMinDistance->setValidator(new QIntValidator(0, 0xffff, this));

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

    maxDistance = ui->maxDistanceEdit->text().toInt();

    ASActive = ui->activateAirScoutcb->isChecked();

    ASActiveBand = static_cast<ASBand>(ui->ASFreqCombo->currentIndex());

    ASServerName = ui->ASServerName->text();
    ASMyName = ui->ASMyName->text();

    ASMaxDistance = ui->ASMaxDistance->text().toInt();
    ASMinDistance = ui->ASMinDistance->text().toInt();


    if (hostname.isEmpty() || port.isEmpty() ||username.isEmpty() ||password.isEmpty() || locator.isEmpty())
        return;



    accept();
}
