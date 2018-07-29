#include "base_pch.h"

#include "ContestApp.h"
#include "tsinglelogframe.h"
#include "tlogcontainer.h"
#include "enqdlg.h"

#include "ScreenConfig.h"
#include "ScreenConfigManager.h"
#include "ui_ScreenConfigManager.h"

ScreenConfigManager::ScreenConfigManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScreenConfigManager)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("ScreenConfigManager/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    scf.loadFile();
    curConfigName = scf.defaultLayoutName;

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (tslf)
        curConfigName = tslf->getCurScreenLayout();
}
int ScreenConfigManager::exec()
{
    showDetails();

    return QDialog::exec();
}
ScreenConfigManager::~ScreenConfigManager()
{
    delete ui;
}
void ScreenConfigManager::showDetails()
{
    suppressItemSelect = true;
    ui->layoutList->clear();

    int crow = -1;

    int j = 0;
    for(QMap<QString, SC>::iterator i = scf.configs.begin(); i != scf.configs.end(); i++ )
    {
        if ((*i).name == curConfigName)
            crow = j;
        ui->layoutList->addItem((*i).name);
        j++;
    }
    ui->layoutList->setCurrentRow(crow);
    checkEnabled();
    repaint();
    suppressItemSelect = false;
}

void ScreenConfigManager::checkEnabled()
{
    bool enable = (curConfigName != scf.defaultLayoutName);

    ui->deleteButton->setEnabled(enable);
    ui->renameButton->setEnabled(enable);
    ui->editButton->setEnabled(enable);
}
void ScreenConfigManager::doCloseEvent()
{
    QSettings settings;
    settings.setValue("ScreenConfigManager/geometry", saveGeometry());
}
void ScreenConfigManager::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void ScreenConfigManager::accept()
{
    doCloseEvent();
    QDialog::accept();
}
void ScreenConfigManager::on_layoutList_itemSelectionChanged()
{
    if (!suppressItemSelect)
    {
        curConfigName = ui->layoutList->currentItem()->text();
        checkEnabled();
    }
}

void ScreenConfigManager::on_layoutList_itemDoubleClicked(QListWidgetItem * /*item*/)
{
    curConfigName = ui->layoutList->currentItem()->text();
    if (curConfigName == scf.defaultLayoutName)
        return;

    on_editButton_clicked();
    checkEnabled();
}
bool ScreenConfigManager::getNewName(QString &Value)
{
    bool firsttime = true;
    while (firsttime || scf.configs.contains(Value))
    {
        firsttime = false;
        if ( enquireDialog( this, "Please give a new name for the layout" , Value ) )
        {
            if (scf.configs.contains(Value))
            {
                MinosParameters::getMinosParameters() ->mshowMessage( Value + " already exists", this );
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }
    return false;
}
void ScreenConfigManager::on_newButton_clicked()
{
    QString value = "new layout";
    if (getNewName(value))
    {
        curConfigName = value;
        SC newsc;
        newsc.name = value;
        scf.configs[curConfigName] = newsc;
        // create empty layout
        showDetails();
    }
}

void ScreenConfigManager::on_cloneButton_clicked()
{
    SC &sc = scf.configs[curConfigName];
    QString value = "new layout";
    if (getNewName(value))
    {
        curConfigName = value;
        // clone current layout
        SC newsc = sc;
        newsc.name = curConfigName;
        scf.configs[curConfigName] = newsc;
        showDetails();
    }
}

void ScreenConfigManager::on_deleteButton_clicked()
{
    scf.configs.remove(curConfigName);
    curConfigName = scf.defaultLayoutName;
    showDetails();

}

void ScreenConfigManager::on_renameButton_clicked()
{
    QString value = curConfigName;
    if (getNewName(value))
    {
        SC sc = scf.configs[curConfigName];
        scf.configs.remove(curConfigName);
        curConfigName = value;
        sc.name = curConfigName;
        scf.configs[curConfigName] = sc;

        // and we need to redo the map
        showDetails();
    }
}

void ScreenConfigManager::on_editButton_clicked()
{
    ScreenConfig sc(this, scf, curConfigName);
    sc.exec();
}

void ScreenConfigManager::on_OKButton_clicked()
{
    on_applyButton_clicked();
    close();
}

void ScreenConfigManager::on_applyButton_clicked()
{
    // write it back, or the screen redraw doesn't work
    scf.dumpFile();

    LogContainer->selectLayout(curConfigName);
    LogContainer->applyScreenLayouts();
}

void ScreenConfigManager::on_cancelButton_clicked()
{
    close();
}
