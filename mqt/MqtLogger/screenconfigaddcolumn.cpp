#include "ScreenConfig.h"

#include "screenconfigaddcolumn.h"
#include "ui_screenconfigaddcolumn.h"

ScreenConfigAddColumn::ScreenConfigAddColumn(ScreenConfig *parent) :
    QDialog(parent),
    sc(parent),
    ui(new Ui::ScreenConfigAddColumn)
{
    ui->setupUi(this);
    ui->topRow->setMinimum(0);
    ui->topRow->setMaximum(sc->topRowCount());
    ui->bottomRow->setMinimum(0);
    ui->bottomRow->setMaximum(sc->topRowCount());
}

ScreenConfigAddColumn::~ScreenConfigAddColumn()
{
    delete ui;
}

void ScreenConfigAddColumn::on_cancelButton_clicked()
{
    reject();
}

void ScreenConfigAddColumn::on_addColumnLeftButton_clicked()
{
    topRow = ui->topRow->value();
    bottomRow = ui->bottomRow->value();
    addLeft = true;

    if (topRow >= bottomRow)
    {
        mShowMessage("Bottom must be at larger than top (i.e. at least two rows", this);
    }
    else
    {
        accept();
    }
}

void ScreenConfigAddColumn::on_addColumnRightButton_clicked()
{
    topRow = ui->topRow->value();
    bottomRow = ui->bottomRow->value();
    addLeft = false;

    if (topRow >= bottomRow)
    {
        mShowMessage("Bottom must be at larger than top (i.e. at least two rows", this);
    }
    else
    {
        accept();
    }
}

