#include "base_pch.h"
#include <QScrollBar>
#include <QScrollArea>
#include "ContestApp.h"
#include "tlogcontainer.h"

#include "ScreenConfigFile.h"
#include "ScreenConfigElement.h"
#include "ScreenConfigRow.h"
#include "ScreenConfigElement.h"
#include "ScreenConfigScreen.h"

#include "ScreenConfig.h"
#include "ui_ScreenConfig.h"
ScreenConfig *screenConfigDialog = nullptr;


ScreenConfig::ScreenConfig(QWidget *parent, ScreenConfigFile &scfp, QString curConfigNamep) :
    QDialog(parent),
    ui(new Ui::ScreenConfig),
    scf(scfp),
    curConfigName(curConfigNamep)
{
    ui->setupUi(this);
    screenConfigDialog = this;

    QSettings settings;
    QByteArray geometry = settings.value("ScreenConfig/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    // create the screen contents based on the config

    SC sc = scf.configs[curConfigName];

    buildScreens(sc);

    curScreen->checkAddButtons();
}

ScreenConfig::~ScreenConfig()
{
    delete ui;
    screenConfigDialog = nullptr;
}
ScreenConfigScreen *ScreenConfig::buildScreens(SC &sc)
{
    ScreenConfigScreen *scr = nullptr;
    for (auto s:sc.baseElement->screens)
    {
        scr = buildScreen(s);
    }
    return scr;
}
ScreenConfigScreen *ScreenConfig::buildScreen(SCScreen &s)
{
    // create a tab in pageTabs, with a scroll area and vertical box layout
    ScreenConfigScreen *scr = new ScreenConfigScreen(this) ;
    scr->setName(s.name);
    scr->mainScreen = s.mainScreen;

    int tno = ui->screenTabs->addTab(scr, s.name);

    ui->screenTabs->setCurrentWidget(ui->screenTabs->widget(tno));
    ui->screenTabs->setTabToolTip(tno, s.name);

    if (s.baseElement)
    {
        scr->buildRows(s.baseElement->rows, scr->baseElement, scr->vbl);
    }

    screens.push_back(scr);
    return scr;
}
bool ScreenConfig::checkOk(ScreenConfigElement *e)
{
    int auxCount = 0;
    int sct = screens.count();
    for (int i = 0; i < sct; i++)
    {
        if (!screens[i]->checkScreenOk(e, auxCount))
        {
            return false;
        }
    }
    QString etype = e->getType();
    if (etype != ScreenConfigElement::getTrScreenTypeString(sctAux) || auxCount < STACKITEMS)
    {
        return true;
    }

    return true;
}
SC ScreenConfig::getConfig()
{
    SC sc;
    sc.name = curConfigName;
    sc.baseElement = QSharedPointer<SCElement>(new SCElement);

    int sct = screens.count();
    for (int i = 0; i < sct; i++)
    {
        sc.baseElement->screens.append(SCScreen());
        SCScreen &scs = sc.baseElement->screens[i] ;
        scs.name = screens[i]->name;

        scs.baseElement = QSharedPointer<SCElement>(new SCElement);
        scs.baseElement->type = ((screens[i]->mainScreen)?sctMainScreen:sctScreen);
        screens[i]->getConfig(scs);
    }
    return sc;
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
    trace("ScreenConfig::on_OKButton_clicked()");

    // analyse and apply the new layout
    SC sc = getConfig();

    // replace it in the config map
    scf.configs[curConfigName] = sc;

    close();
}

void ScreenConfig::on_applyButton_clicked()
{
    // First, analyse the screen layout into a config object
    trace("ScreenConfig::on_applyButton_clicked()");

    SC sc = getConfig();

    // replace it in the config map
    scf.configs[curConfigName] = sc;


    // write it back, or the screen redraw doesn't work
    scf.dumpFile();

    LogContainer->selectLayout(curConfigName);
    LogContainer->selectSession(TContestApp::getContestApp()->currSession);
}

void ScreenConfig::on_cancelButton_clicked()
{
    trace("ScreenConfig::on_cancelButton_clicked()");
    close();
}

void ScreenConfig::on_addScreenButton_clicked()
{
    trace("ScreenConfig::on_addScreenButton_clicked()");
    // create a new screen, and its tab, switch to it, and add a row

    SCScreen s;
    buildScreen(s);
    curScreen->on_addRowButton_clicked();
}


void ScreenConfig::on_screenTabs_currentChanged(int index)
{
    if (index == -1)
    {
        return;
    }
    else
    {
        QWidget *ctab = ui->screenTabs->widget(index);
        curScreen = dynamic_cast<ScreenConfigScreen *>(ctab);
    }
}
void ScreenConfig::setScreenName(ScreenConfigScreen *scr)
{
    ui->screenTabs->setTabText(ui->screenTabs->currentIndex(), scr->name);


}
