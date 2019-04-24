#include "base_pch.h"
#include <QScrollBar>

#include "ScreenConfigElement.h"
#include "ScreenConfigRow.h"
#include "ScreenConfig.h"

#include "ui_ScreenConfigRow.h"

ScreenConfigRow::ScreenConfigRow(ScreenConfigElement *parentc) :
    QFrame(nullptr)
  , ui(new Ui::ScreenConfigRow)
  , parentElement(parentc)
{
    ui->setupUi(this);
    hbl = new QHBoxLayout(ui->scrollAreaWidgetContents);
    hbl->setMargin(1);
    ui->scrollAreaWidgetContents->setLayout(hbl);
}

ScreenConfigRow::~ScreenConfigRow()
{
    delete ui;
}

void ScreenConfigRow::on_addRowBeforeButton_clicked()
{
    trace("ScreenConfigRow::on_addRowBeforeButton_clicked()");
    parentElement->addRowBefore(this);
}

void ScreenConfigRow::on_removeRowButton_clicked()
{
    trace("ScreenConfigRow::on_removeRowButton_clicked()");
    parentElement->removeRow(this);
}

void ScreenConfigRow::on_addRowAfterButton_clicked()
{
    trace("ScreenConfigRow::on_addRowAfterButton_clicked()");
    parentElement->addRowAfter(this);
}

ScreenConfigElement * ScreenConfigRow::addLeft(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < hbl->count(); i++)
    {
        if (hbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigElement *baseElement = new ScreenConfigElement(this);
    hbl->insertWidget( pos, baseElement);

    return baseElement;
}
void ScreenConfigRow::remove(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < hbl->count(); i++)
    {
        if (hbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    QLayoutItem *taken = hbl->takeAt(pos);
    if (taken)
    {
        // From the source, I don't think the deleting the layout item deletes the widget
        taken->widget()->deleteLater();
        delete taken;
    }
    if (hbl->count() == 0)
    {
        parentElement->removeRow(this);
    }
}

ScreenConfigElement *ScreenConfigRow::addRight(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < hbl->count(); i++)
    {
        if (hbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigElement *baseElement = new ScreenConfigElement(this);
    hbl->insertWidget( pos + 1, baseElement);

    return baseElement;
}

bool ScreenConfigRow::checkOk(ScreenConfigElement *e)
{
    return parentElement->checkOk(e);
}
