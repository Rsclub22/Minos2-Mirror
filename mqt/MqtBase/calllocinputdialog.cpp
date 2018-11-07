#include "calllocinputdialog.h"
#include "ui_calllocinputdialog.h"


CallLocInputDialog::CallLocInputDialog(QWidget *parent, QString textToEdit, const QString dialogTitle, const QString lineEditLable) :
    QDialog(parent),
    ui(new Ui::CallLocInputDialog)
{
    ui->setupUi(this);
    setWindowTitle(dialogTitle);
    ui->lineEditLabel->setText(lineEditLable);
    ui->lineEdit->setValidator(new UpperCaseValidator(true));
    ui->lineEdit->setText(textToEdit);

}

CallLocInputDialog::~CallLocInputDialog()
{
    delete ui;
}



QString CallLocInputDialog::getText()
{
    return ui->lineEdit->text().trimmed();
}
