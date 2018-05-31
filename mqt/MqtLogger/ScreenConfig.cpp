#include "base_pch.h"
#include <QScrollBar>

#include "ContestApp.h"
#include "tlogcontainer.h"

#include "ScreenConfigFile.h"
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

    // create the screen contents based on the config

    scf.loadFile();

    curConfigName = "default";

    SC sc = scf.configs[curConfigName];

    for (int j = 0; j < sc.rows.count(); j++)
    {
        ScreenConfigRow *baseRow = new ScreenConfigRow(parentWidget(), this);
        vbl->insertWidget( j, baseRow);
       for (int k = 0; k < sc.rows[j].elements.count(); k++)
       {
           ScreenConfigElement *e = new ScreenConfigElement(this, baseRow);
           e->setType(sc.rows[j].elements[k].type);
           baseRow->vbl->insertWidget(k, e);
       }
    }
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
SC ScreenConfig::getConfig()
{
    SC sc;
    sc.name = curConfigName;

    for (int i = 0; i < vbl->count(); i++)
    {
        QWidget *w = vbl->itemAt(i)->widget();
        ScreenConfigRow *row = dynamic_cast<ScreenConfigRow *>(w);
        if (row)
        {
            SCRow scrow;
            for (int j = 0; j < row->vbl->count(); j++)
            {
                w = row->vbl->itemAt(j)->widget();
                ScreenConfigElement *ele = dynamic_cast<ScreenConfigElement *>(w);
                if (ele)
                {
                    SCElement scele;
                    scele.type = ele->getType();
                    scrow.elements.append(scele);

                }
            }
            sc.rows.append(scrow);
        }
    }
    return sc;
}
void ScreenConfig::on_OKButton_clicked()
{
    // analyse and apply the new layout
    on_applyButton_clicked();

    close();
}

void ScreenConfig::on_applyButton_clicked()
{
    // First, analyse the screen layout into a config object

    SC sc = getConfig();

    // replace it in the config map
    scf.configs[curConfigName] = sc;


    // write it back, or the screen redraw doesn't work
    scf.dumpFile();

    // and apply it to the open logs
    TContestApp *app = TContestApp::getContestApp();
    QString sessName = app->currSession;

    LogContainer->closeSession();

    // clear old splitter settings
    QSettings settings;
    settings.remove("logFrameSplitter");

    for (int i = 0; i < sc.rows.count(); i++)
    {
        QString name = "row" + QString::number(i) + "splitter";

        settings.remove(name);
    }

    LogContainer->selectSession(sessName);
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
