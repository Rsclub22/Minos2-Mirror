#include "base_pch.h"
#include <QScrollBar>

#include "ScreenConfigElement.h"
#include "ScreenConfigRow.h"

#include "ScreenConfig.h"
#include "ui_ScreenConfig.h"

ScreenConfig::ScreenConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScreenConfig)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("ScreenConfig/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    vbl = new QVBoxLayout(ui->scrollAreaWidgetContents);
    vbl->setMargin(1);
    ui->scrollAreaWidgetContents->setLayout(vbl);

//    ScreenConfigRow *baserow = new ScreenConfigRow(parent, this);
//    vbl->addWidget(baserow);

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

ScreenConfig::~ScreenConfig()
{
    delete ui;
}
void ScreenConfig::doCloseEvent()
{
    QSettings settings;
    settings.setValue("ScreenConfig/geometry", saveGeometry());
}
void ScreenConfig::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void ScreenConfig::accept()
{
    doCloseEvent();
    QDialog::accept();
}
void ScreenConfig::on_OKButton_clicked()
{
    close();
}

void ScreenConfig::on_applyButton_clicked()
{

}

void ScreenConfig::on_cancelButton_clicked()
{
    close();
}
void ScreenConfig::addBefore(ScreenConfigRow *r)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == r)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigRow *baseRow = new ScreenConfigRow(parentWidget(), this);
    vbl->insertWidget( pos, baseRow);
    baseRow->addLeft(nullptr);

}
void ScreenConfig::remove(ScreenConfigRow *r)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == r)
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
}
void ScreenConfig::addAfter(ScreenConfigRow *r)
{
    int pos = 0;
    for (int i = 0; i < vbl->count(); i++)
    {
        if (vbl->itemAt(i)->widget() == r)
        {
            pos = i;
            break;
        }
    }
    ScreenConfigRow *baseRow = new ScreenConfigRow(parentWidget(), this);
    vbl->insertWidget( pos + 1, baseRow);
    baseRow->addLeft(nullptr);
}

void ScreenConfig::on_addRowButton_clicked()
{
    QLayoutItem *last = nullptr;
    QWidget *wlast = nullptr;

    if (vbl->count())
    {
        last = vbl->itemAt(vbl->count() - 1);
        wlast = last->widget();
    }

    addAfter(dynamic_cast<ScreenConfigRow *>(wlast));
}
