#include "base_pch.h"

#include "ContestApp.h"
#include "tsinglelogframe.h"
#include "tlogcontainer.h"
#include "enqdlg.h"

#include "ScreenConfig.h"
#include "ScreenConfigManager.h"
#include "ui_ScreenConfigManager.h"

const char * ScreenConfigManager::defLayoutText = QT_TR_NOOP("(default)");
const char * ScreenConfigManager::protectedLayoutText = QT_TR_NOOP("(protected)");

ScreenConfigManager::ScreenConfigManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScreenConfigManager),
    scf(ScreenConfigFile::getScreenConfigFile(parent))
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("ScreenConfigManager/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    scf.loadFile(this);
    curConfigName = defaultLayoutName();

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (tslf)
        curConfigName = tslf->getCurScreenLayout();

    MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpDefaultLayout, defaultConfigName );
    MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpProtectedLayout, protectedConfigName );
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
    for(auto const &i: scf.configs )
    {
        if (i.name == curConfigName)
            crow = j;

        QString postText;
        if (i.name ==  defaultConfigName)
        {
            postText = tr(defLayoutText);
        }
        if (i.name ==  protectedConfigName)
        {
            postText += tr(protectedLayoutText);
        }

        if (postText.isEmpty())
        {
            ui->layoutList->addItem(i.name);
        }
        else
        {
            ui->layoutList->addItem(i.name + " " + postText);
        }

        j++;
    }
    ui->layoutList->setCurrentRow(crow);
    checkEnabled();
    update();
    suppressItemSelect = false;
}

void ScreenConfigManager::checkEnabled()
{
    QString dln = defaultLayoutName();
    QString dpln = defaultProtectedLayoutName();
    bool enable = (curConfigName != dln && curConfigName != dpln);

    ui->deleteButton->setEnabled(enable);
    ui->renameButton->setEnabled(enable);
    ui->editButton->setEnabled(enable);

    enable = (curConfigName != defaultLayoutName());
    ui->protectedButton->setEnabled(enable);

    enable = (curConfigName != defaultProtectedLayoutName());
    ui->makeDefaultButton->setEnabled(enable);
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
QString ScreenConfigManager::stripDefaultDecoration(QString s)
{
    if (s.endsWith(tr(ScreenConfigManager::protectedLayoutText)))
    {
        s.chop(tr(ScreenConfigManager::protectedLayoutText).size());
        s = s.trimmed();
    }
    if (s.endsWith(tr(ScreenConfigManager::defLayoutText)))
    {
        s.chop(tr(ScreenConfigManager::defLayoutText).size());
        s = s.trimmed();
    }
    return s;
}
void ScreenConfigManager::on_layoutList_itemSelectionChanged()
{
    if (!suppressItemSelect)
    {
        curConfigName = stripDefaultDecoration(ui->layoutList->currentItem()->text());
        checkEnabled();
    }
}

void ScreenConfigManager::on_layoutList_itemDoubleClicked(QListWidgetItem * /*item*/)
{
    curConfigName = stripDefaultDecoration(ui->layoutList->currentItem()->text());
    if (curConfigName == defaultLayoutName())
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
        if ( enquireDialog( this, tr("Please give a new name for the layout") , Value ) )
        {
            if (scf.configs.contains(Value))
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
void ScreenConfigManager::on_newButton_clicked()
{
    QString value =tr("new layout");
    if (getNewName(value))
    {
        curConfigName = value;
        SC newsc;
        newsc.name = value;
        scf.configs[curConfigName] = newsc;
        // create empty layout
        showDetails();
        on_editButton_clicked();
    }
}

void ScreenConfigManager::on_cloneButton_clicked()
{
    SC &sc = scf.configs[curConfigName];
    QString value = tr("new layout");
    if (getNewName(value))
    {
        curConfigName = value;
        // clone current layout
        SC newsc = sc;
        newsc.name = curConfigName;
        scf.configs[curConfigName] = newsc;
        showDetails();
        on_editButton_clicked();
    }
}

void ScreenConfigManager::on_deleteButton_clicked()
{
    scf.configs.remove(curConfigName);
    if (curConfigName == defaultConfigName)
    {
        defaultConfigName = defaultLayoutName();
    }
    if (curConfigName == protectedConfigName)
    {
        protectedConfigName = defaultProtectedLayoutName();
    }
    curConfigName = defaultConfigName;
    showDetails();

}

void ScreenConfigManager::on_renameButton_clicked()
{
    QString oldName = curConfigName;
    QString value = curConfigName;
    if (getNewName(value))
    {
        SC sc = scf.configs[curConfigName];
        scf.configs.remove(curConfigName);
        curConfigName = value;
        sc.name = curConfigName;
        scf.configs[curConfigName] = sc;

        if (defaultConfigName == oldName)
        {
            defaultConfigName = curConfigName;
        }
        if (protectedConfigName == oldName)
        {
            protectedConfigName = curConfigName;
        }

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
    MinosParameters::getMinosParameters() -> setStringDisplayProfile( edpDefaultLayout, defaultConfigName );
    MinosParameters::getMinosParameters() -> setStringDisplayProfile( edpProtectedLayout, protectedConfigName );
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

void ScreenConfigManager::on_makeDefaultButton_clicked()
{
    defaultConfigName = curConfigName;
    MinosParameters::getMinosParameters() -> setStringDisplayProfile( edpDefaultLayout, curConfigName );
    showDetails();
}

void ScreenConfigManager::on_protectedButton_clicked()
{
    protectedConfigName = curConfigName;
    MinosParameters::getMinosParameters() -> setStringDisplayProfile( edpProtectedLayout, curConfigName );
    showDetails();
}
