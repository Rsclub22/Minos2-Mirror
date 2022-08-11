#include <QSettings>

#include "ConfigFile.h"
#include "StartConfig.h"
#include "enqdlg.h"
#include "delayedaction.h"
#include "MinosParameters.h"

#include "StartConfigManager.h"
#include "ui_StartConfigManager.h"

StartConfigManager::StartConfigManager(QWidget *parent, bool showAutoStart) :
    QDialog(parent),
    ui(new Ui::StartConfigManager)
  , showAutoStart(showAutoStart)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    curConfigName = minosConfig->getCurrConfig().configName;

    QSettings settings;
    QByteArray geometry = settings.value("ManageAppConfigs/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

}

StartConfigManager::~StartConfigManager()
{
    delete ui;
}

int StartConfigManager::exec()
{
    showDetails();

    return QDialog::exec();
}
void StartConfigManager::checkEnabled()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    bool running = minosConfig->anyRunning();

    bool enable = !running && minosConfig->configs.size() > 0;

    ui->newButton->setEnabled(enable);
    ui->cloneButton->setEnabled(enable);
    ui->deleteButton->setEnabled(enable);
    ui->renameButton->setEnabled(enable);
    ui->editButton->setEnabled(enable);

    if (running)
    {
        ui->startStopButton->setText(tr("Stop all apps"));
    }
    else
    {
        ui->startStopButton->setText(tr("Start all apps"));
        ui->startStopButton->setEnabled(enable);
    }
}

void StartConfigManager::showDetails()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();

    suppressItemSelect = true;
    ui->layoutList->clear();

    int crow = -1;

    int j = 0;
    for(auto const &i: qAsConst(minosConfig->configs ))
    {
        if (i.configName == curConfigName)
           crow = j;

        ui->layoutList->addItem(i.configName);

        j++;
    }
    ui->layoutList->setCurrentRow(crow);
    checkEnabled();
    update();
    suppressItemSelect = false;
}

void StartConfigManager::on_cancelButton_clicked()
{
    reject();
}

void StartConfigManager::on_OKButton_clicked()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    minosConfig->saveAll();
    accept();
}
void StartConfigManager::reject()
{
    bool running = MinosConfig::getMinosConfig() ->anyRunning();
    if (!running)
        MinosConfig::getMinosConfig() ->reset();
    QDialog::reject();
}
void StartConfigManager::accept()
{
    QDialog::accept();
}
void StartConfigManager::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("ManageAppConfigs/geometry", saveGeometry());
    QDialog::moveEvent(event);
}
void StartConfigManager::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("ManageAppConfigs/geometry", saveGeometry());
    QDialog::resizeEvent(event);
}
void StartConfigManager::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("ManageAppConfigs/geometry", saveGeometry());
    }
}
bool StartConfigManager::getNewName(QString &Value)
{
    bool firsttime = true;
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();

    while (firsttime || minosConfig->configs.contains(Value) || Value.isEmpty())
    {
        firsttime = false;
        if ( enquireDialog( this, tr("Please give a new name for the layout") , Value ) )
        {
            if (minosConfig->configs.contains(Value))
            {
                MinosParameters::getMinosParameters() ->mshowMessage(tr("%1 already exists").arg(Value), this );
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
void StartConfigManager::on_editButton_clicked()
{
    StartConfig startConfig(this, showAutoStart, curConfigName);
    startConfig.exec();

}

void StartConfigManager::on_newButton_clicked()
{
    QString value =tr("new app config");
    if (getNewName(value))
    {
        MinosConfig *minosConfig = MinosConfig::getMinosConfig();
        curConfigName = value;
        NamedConfig newsc;
        newsc.configName = value;
        minosConfig->configs[curConfigName] = newsc;
        minosConfig->setCurConfig(curConfigName);
        // create empty layout
        showDetails();
        on_editButton_clicked();
    }
}

void StartConfigManager::on_cloneButton_clicked()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    NamedConfig &sc = minosConfig->configs[curConfigName];
    QString value = tr("new app config");
    if (getNewName(value))
    {
        curConfigName = value;
        // clone current layout
        NamedConfig newsc = sc;
        newsc.configName = curConfigName;
        minosConfig->configs[curConfigName] = newsc;
        minosConfig->setCurConfig(curConfigName);
        showDetails();
        on_editButton_clicked();
    }
}

void StartConfigManager::on_deleteButton_clicked()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    minosConfig->configs.remove(curConfigName);

    if (minosConfig->configs.size())
    {
        curConfigName = minosConfig->configs.begin()->configName;
    }
    else
    {
        curConfigName.clear();
    }
    minosConfig->setCurConfig(curConfigName);
    showDetails();

}

void StartConfigManager::on_renameButton_clicked()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    QString value = curConfigName;
    if (getNewName(value))
    {
        NamedConfig sc = minosConfig->configs[curConfigName];
        minosConfig->configs.remove(curConfigName);
        curConfigName = value;
        sc.configName = curConfigName;
        minosConfig->configs[curConfigName] = sc;
        minosConfig->setCurConfig(curConfigName);

        // and we need to redo the map
        showDetails();
    }
}

void StartConfigManager::on_layoutList_itemSelectionChanged()
{
    if (!suppressItemSelect)
    {
        MinosConfig *minosConfig = MinosConfig::getMinosConfig();
        curConfigName = ui->layoutList->currentItem()->text();
        minosConfig->setCurConfig(curConfigName);
    }
    checkEnabled();
}

void StartConfigManager::on_layoutList_itemDoubleClicked(QListWidgetItem * /*item*/)
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    curConfigName = ui->layoutList->currentItem()->text();
    minosConfig->setCurConfig(curConfigName);

    on_editButton_clicked();
    checkEnabled();
}

void StartConfigManager::on_startStopButton_clicked()
{
    // start (or stop) as appropriate
    // if start, close this dialog
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    bool running = minosConfig->anyRunning();

    ui->startStopButton->setEnabled(false);
    if (running)
    {
        MinosConfig::getMinosConfig() ->askStop();
        MinosConfig::getMinosConfig() ->forceStop();
        delayedAction(this, [=](){
            checkEnabled();
        });
    }
    else
    {
        minosConfig->saveAll();
        minosConfig->start();
        accept();
    }
}
