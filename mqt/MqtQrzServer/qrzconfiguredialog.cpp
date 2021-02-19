#include "qrzconfiguredialog.h"
#include "ui_qrzconfiguredialog.h"

QrzConfigureDialog::QrzConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QrzConfigureDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){onAccepted();});
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [=](){onRejected();});

}

QrzConfigureDialog::~QrzConfigureDialog()
{
    delete ui;
}


void QrzConfigureDialog::onRejected()
{
    reject();
}

void QrzConfigureDialog::onAccepted()
{

    logCallsign = ui->logonCallsignLineEdit->text().trimmed();
    logPassword = ui->qrzPasswordLineEdit->text().trimmed();

    accept();
}


int QrzConfigureDialog::exec()
{
    ui->logonCallsignLineEdit->setText(logCallsign);
    ui->qrzPasswordLineEdit->setText(logPassword);

    return QDialog::exec();
}
