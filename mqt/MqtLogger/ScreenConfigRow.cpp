#include "base_pch.h"
#include <QScrollBar>

#include "ScreenConfigElement.h"
#include "ScreenConfigRow.h"
#include "ScreenConfig.h"

#include "ui_ScreenConfigRow.h"

ScreenConfigRow::ScreenConfigRow(QWidget *parent, ScreenConfig *parentc) :
    QFrame(parent)
  , ui(new Ui::ScreenConfigRow)
  , parentDialog(parentc)
{
    ui->setupUi(this);
    vbl = new QHBoxLayout(ui->scrollAreaWidgetContents);
    vbl->setMargin(1);
    ui->scrollAreaWidgetContents->setLayout(vbl);

 //   ScreenConfigElement *baseElement = new ScreenConfigElement(parentWidget(), this);
 //   vbl->addWidget(baseElement);


//    elementFrames.clear();
//    MinosConfig *minosConfig = MinosConfig::getMinosConfig();

//    int offset = 0;
//    for (int i = 0; i <  minosConfig->elelist.size(); i++)
//    {
//        QSharedPointer<RunConfigElement> c = minosConfig->elelist[i];
//        if (c->deleted)
//            continue;

//        ConfigElementFrame *cef = new ConfigElementFrame(false);

        // set alternating background

//        vbl->addWidget(cef);

//        cef->setElement(c);
//        elementFrames.append(cef);
//    }

}

ScreenConfigRow::~ScreenConfigRow()
{
    delete ui;
}

void ScreenConfigRow::on_addBeforeButton_clicked()
{
    parentDialog->addBefore(this);
}

void ScreenConfigRow::on_removeButton_clicked()
{
    parentDialog->remove(this);
}

void ScreenConfigRow::on_addAfterButton_clicked()
{
    parentDialog->addAfter(this);
}

void ScreenConfigRow::addLeft(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigElement *baseElement = new ScreenConfigElement(parentWidget(), this);
    vbl->insertWidget( pos, baseElement);

}
void ScreenConfigRow::remove(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    QLayoutItem *taken = vbl->takeAt(pos);
    if (taken)
    {
        // From the source, I don't think the deleting the layout item deletes the widget
        taken->widget()->deleteLater();
        delete taken;
    }
    if (vbl->count() == 0)
    {
        parentDialog->remove(this);
    }
}
void ScreenConfigRow::addRight(ScreenConfigElement *e)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == e)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigElement *baseElement = new ScreenConfigElement(parentWidget(), this);
    vbl->insertWidget( pos + 1, baseElement);

}
