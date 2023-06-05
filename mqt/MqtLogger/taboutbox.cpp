#include <QDesktopServices>
#include "regsettings.h"
#include "MShowMessageDlg.h"
#include "SecondInstall.h"
#include "ContestApp.h"
#include "tlogcontainer.h"
#include "TSessionManager.h"
#include "StartConfig.h"
#include "ConfigFile.h"
#include "StartConfigManager.h"
#include "ServerEvent.h"
#include "taboutbox.h"
#include "ui_taboutbox.h"

QString TAboutBox::creditsText = QString(
    "<br><center><h2>%1</h2>"

    "<br><h3>%2</h3>"
    "<br>Mike Goodey G0GJV; David Balharrie M0DGB/G8FKH"
    "<br><h3>%3</h3>"
    "<br>Ken Punshon G4APJ"
    "<br><h3>%4</h3>"
    "<br>Ken Punshon G4APJ"
    "<br><h3>%5</h3>"
    "Phil Taylor M0VSE, Andy Cunningham M0HAK"
    "<br><h3>%6</h3>"
    "<br> Neil Yorke M0NKE"
    "<br><h3>%7</h3>"
    "<br> Peter Burton G3ZPB"
    "<br><h3>%8</h3>"
    "<br>Ken Punshon G4APJ, Jacques Lepoil F1BHL"
    "<br><h3>%9</h3>"
    "<br>Dave Sergeant G3YMC"
    "<br><h3>%10</h3>"
    "<br>Jacqui Goodey G6XSY, Marie Balharrie, Sue Punshon"
     );
QString TAboutBox::MinosText =
   "The Inferno, Dante, (trans Dorothy L Sayers, Penguin Classics); Canto 5.\r\n"
   "\r\n"
   "\"From the first circle thus I came descending\r\n"
   "To the second, which, in narrower compass turning,\r\n"
   "Holds greater woe, with outcry loud and rending.\"\r\n"
   "\r\n"
   "\"There in the threshold, horrible and giming,\r\n"
   "Grim Minos sits, holding his ghastly session,\r\n"
   "And, as he girds him, sentencing and spurning;\"\r\n"
   "\r\n"
   "\"For when the ill soul faces him, confession\r\n"
   "Pours out of it till nothing's left to tell;\r\n"
   "Whereon that connoisseur of all transgression\"\r\n"
   "\r\n"
   "\"Assigns it to its proper place in hell,\r\n"
   "As many grades as he would have it fall,\r\n"
   "So oft he belts him round with his own tail.\"\r\n"
   "\r\n"
   "\"Before him stands a throng continual;\r\n"
   "Each comes in turn to abye the fell arraign;\r\n"
   "They speak - they hear - they're whirled down one and all.\"\r\n"
   "\r\n"
   "So when I was thinking a name for a messaging server, it amused me to think of it belting messages round a field with its tail!\r\n"
   "\r\n"
   "Of course another quote from Dante is most apt for anyone thinking of becoming involved in this kind of project…\r\n"
   "\r\n"
   "“Abandon Hope, all ye who enter here”…"
   ;

/*static*/bool TAboutBox::ShowAboutBox(QWidget *Owner,  bool onStartup )
{
   TAboutBox aboutBox( Owner, onStartup);


   int ret = aboutBox.exec();

   return ret == QDialog::Accepted;
}

int TAboutBox::exec()
{
    int ret = QDialog::exec();
    if (ret == QDialog::Rejected)
    {
        doStartup = false;
    }
    else
    {
        LogContainer->setCurrSessionName( ui->chooseSetCb->currentText());
    }
    if ( doStartup )
    {
       // auto start on first run, but only if we gave that option

       QString reqErrs = MinosConfig::getMinosConfig() ->checkConfig();

       if (reqErrs.isEmpty())
            MinosConfig::getMinosConfig() ->start();
       else
           mShowMessage(reqErrs, this);
    }
    return ret;
}

TAboutBox::TAboutBox(QWidget *parent, bool onStartup) :
    QDialog(parent),
    ui(new Ui::TAboutBox),
    doStartup(onStartup)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("MinosAbout/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->PageControl1->setCurrentWidget(ui->AboutTabSheet);

    QString welcomeText = QString("<br><h1>%1 ") + STRINGVERSION  + " " + PRERELEASETYPE + " " + SecondInstall::getSecondInstallText() + "</h1>"
                                   "<br>Home page"
                                   "<br><a href=\"http://minos.sourceforge.net/\">http://minos.sourceforge.net</a>"
                                   "<br><br>User forum (please join!)"
                                   "<br><a href=\"https://minos.groups.io/g/users\">https://minos.groups.io/g/users</a>"
                                   "<br><br><h2><a href=\"file://Manual\">Click here to read the manual!</a>"
                                   "<br><br>";
    ui->AboutMemo->setText(welcomeText.arg(tr("Welcome to Minos Version")));
    ui->AboutMemo->setTextFormat(Qt::RichText);
    ui->AboutMemo->setTextInteractionFlags(Qt::TextBrowserInteraction);

    ui->creditsLabel->setTextFormat(Qt::RichText);

    ui->creditsLabel->setText(creditsText
                       .arg(tr("Credits"))
                       .arg(tr("Main Developers"))
                       .arg(tr("Testing"))
                       .arg(tr("Documentation"))
                       .arg(tr("MAC port"))
                       .arg(tr("Icons"))
                       .arg(tr("Rig testing"))
                       .arg(tr("French Translation"))
                       .arg(tr("Web Site"))
                       .arg(tr("And last (but not least) our long suffering wives"))
                       );

    ui->MinosMemo->setText(MinosText);

    ui->ExitButton->setVisible(onStartup);
    ui->LoggerOnlyButton->setVisible(onStartup);
    ui->SessionsFrame->setVisible(onStartup);

    if (onStartup)
    {
        TContestApp *app = TContestApp::getContestApp();

        // This sets app ->currSession which allows an incoming
        // LanguageChange even to do things...

        QString sess = LogContainer->getCurrSession();
        if (sess.isEmpty())
        {
            sess = app->defaultSession;
        }

        // so clear it again!
        app ->currSession.clear();

        QStringList sessionlst = LogContainer->getSessions();
        if (sessionlst.count())
        {
            ui->chooseSetCb->addItems(sessionlst);
        }
        else
        {
            ui->chooseSetCb->addItem(sess);
        }
        ui->chooseSetCb->setCurrentText(sess);

        int cap;
        TContestApp::getContestApp() ->loggerBundle.getIntProfile(elpAgeToProtectContests, cap);
        bool doAge;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAgeProtectContests, doAge );

        if (cap == -1)
        {
            cap = 0;
            doAge = false;
            TContestApp::getContestApp() ->loggerBundle.setBoolProfile(elpAgeProtectContests, doAge);
            TContestApp::getContestApp() ->loggerBundle.setIntProfile(elpAgeToProtectContests, cap);
            TContestApp::getContestApp() ->loggerBundle.flushProfile();
        }

        ui->ageSpinner->setValue(cap);

        ui->ageCb->setChecked(doAge);

        ui->ageSpinner->setEnabled(ui->ageCb->isChecked());
    }

    if (  onStartup && !checkRouterReady() )
    {
        doStartup = true; // click the start button on form close
    }
}

TAboutBox::~TAboutBox()
{
    delete ui;
}
void TAboutBox::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("MinosAbout/geometry", saveGeometry());
}
void TAboutBox::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TAboutBox::accept()
{
    doCloseEvent();
    QDialog::accept();
}
void TAboutBox::on_AboutMemo_linkActivated(const QString &link)
{
    if (link == "file://Manual")
    {
        LogContainer->HelpActionExecute();
    }
    else
    {
        QDesktopServices::openUrl(QUrl(link));
    }
}

void TAboutBox::on_ExitButton_clicked()
{
    doStartup = false;
    reject();
}

void TAboutBox::on_OKButton_clicked()
{
    accept();
}

void TAboutBox::on_LoggerOnlyButton_clicked()
{
    doStartup = false;
    accept();
}


void TAboutBox::on_AppsButton_clicked()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    StartConfig configBox( this, false, minosConfig->getCurrConfig().configName);
    configBox.exec();
}
void TAboutBox::on_appSelectButton_clicked()
{
    StartConfigManager manageApps( this, true);   // when managing sets, include autostart
    manageApps.exec();
}

void TAboutBox::on_manageSets_clicked()
{
    LogContainer->setCurrSessionName(ui->chooseSetCb->currentText());
    QString sess = LogContainer->getCurrSession();  // sets ap->currSession
    TSessionManager tsm(this);
    tsm.exec();

    ui->chooseSetCb->clear();

    // it might have changed
    sess = LogContainer->getCurrSession();

    QStringList sessionlst = LogContainer->getSessions();
    ui->chooseSetCb->addItems(sessionlst);
    ui->chooseSetCb->setCurrentText(sess);
}

void TAboutBox::on_ageSpinner_valueChanged(int cap)
{
    TContestApp::getContestApp() ->loggerBundle.setIntProfile(elpAgeToProtectContests, cap);
    TContestApp::getContestApp() ->loggerBundle.flushProfile();
}

void TAboutBox::on_ageCb_stateChanged(int /*arg1*/)
{
    ui->ageSpinner->setEnabled(ui->ageCb->isChecked());
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile(elpAgeProtectContests, ui->ageCb->isChecked());
    TContestApp::getContestApp() ->loggerBundle.flushProfile();
}

