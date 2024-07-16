#include "managehamlib.h"
#include "ui_managehamlib.h"

ManageHamlib::ManageHamlib(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManageHamlib)
{
    ui->setupUi(this);
}

ManageHamlib::~ManageHamlib()
{
    delete ui;
}

void ManageHamlib::on_cancelButton_clicked()
{
    reject();
}

