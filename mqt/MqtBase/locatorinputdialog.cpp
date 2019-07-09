#include "locatorinputdialog.h"
#include "ui_locatorinputdialog.h"

LocatorInputDialog::LocatorInputDialog(QWidget *parent, QString textToEdit, const QString dialogTitle, const QString lineEditLabel) :
    QDialog(parent),
    ui(new Ui::LocatorInputDialog)

{
    ui->setupUi(this);
    setWindowTitle(dialogTitle);
    ui->locatorInput->setText(lineEditLabel);
    ui->locatorInput->setValidator(new UpperCaseValidator());
    ui->locatorInput->setText(textToEdit);
}

LocatorInputDialog::~LocatorInputDialog()
{
    delete ui;
}



QString LocatorInputDialog::getText()
{
    return ui->locatorInput->text().trimmed();
}

void LocatorInputDialog::setText(QString text)
{
    ui->locatorInput->setText(text);
}

bool LocatorInputDialog::isValid()
{
    return ui->locatorInput->isValid();
}

void LocatorInputDialog::allowLoc4(bool allow)
{
    ui->locatorInput->setAllowLoc4(allow);
}
