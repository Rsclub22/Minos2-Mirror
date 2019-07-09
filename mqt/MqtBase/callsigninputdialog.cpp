#include "callsigninputdialog.h"
#include "ui_callsigninputdialog.h"

CallsignInputDialog::CallsignInputDialog(QWidget *parent, QString textToEdit, const QString dialogTitle, const QString lineEditLabel) :
    QDialog(parent),
    ui(new Ui::CallsignInputDialog)
{
    ui->setupUi(this);
    setWindowTitle(dialogTitle);
    ui->callsignInput->setText(lineEditLabel);
    ui->callsignInput->setValidator(new UpperCaseValidator());
    ui->callsignInput->setText(textToEdit);
}

CallsignInputDialog::~CallsignInputDialog()
{
    delete ui;
}

QString CallsignInputDialog::getText()
{
    return ui->callsignInput->text().trimmed();
}

void CallsignInputDialog::setText(QString text)
{
    ui->callsignInput->setText(text);
}

bool CallsignInputDialog::isValid()
{
    return ui->callsignInput->isValid();
}
