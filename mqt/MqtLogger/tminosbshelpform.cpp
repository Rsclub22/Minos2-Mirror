#include "tminosbshelpform.h"
#include "ui_tminosbshelpform.h"

TMinosBSHelpForm::TMinosBSHelpForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TMinosBSHelpForm)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

TMinosBSHelpForm::~TMinosBSHelpForm()
{
    delete ui;
}
void TMinosBSHelpForm::setText(const QString &text)
{
    ui->helpText->setText(text);
}

void TMinosBSHelpForm::on_CloseButton_clicked()
{
    accept();
}
