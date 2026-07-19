#include <QDesktopServices>
#include <QUrl>
#include <QFontDialog>
#include <QFont>
#include "AppStartup.h"
#include "MTrace.h"
//#include "LoggerContest.h"
#include "managecontestsettings.h"
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
#include "MinosLoggerEvents.h"
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
    "<br> Peter Burton G3ZPB (SK)"
    "<br><h3>%8</h3>"
    "<br>Ken Punshon G4APJ, Jacques Lepoil F1BHL"
    "<br><h3>%9</h3>"
    "<br>Dave Sergeant G3YMC"
    "<br><h3>%10</h3>"
    "<br>Jacqui Goodey G6XSY, Marie Balharrie (RIP), Sue Punshon"
     );
QString TAboutBox::MinosText =
   "The Inferno, Dante, (trans Dorothy L Sayers, Penguin Classics); Canto 5.\n"
   "\n"
   "\"From the first circle thus I came descending\n"
   "To the second, which, in narrower compass turning,\n"
   "Holds greater woe, with outcry loud and rending.\"\n"
   "\n"
   "\"There in the threshold, horrible and giming,\n"
   "Grim Minos sits, holding his ghastly session,\n"
   "And, as he girds him, sentencing and spurning;\"\n"
   "\n"
   "\"For when the ill soul faces him, confession\n"
   "Pours out of it till nothing's left to tell;\n"
   "Whereon that connoisseur of all transgression\"\n"
   "\n"
   "\"Assigns it to its proper place in hell,\n"
   "As many grades as he would have it fall,\n"
   "So oft he belts him round with his own tail.\"\n"
   "\n"
   "\"Before him stands a throng continual;\n"
   "Each comes in turn to abye the fell arraign;\n"
   "They speak - they hear - they're whirled down one and all.\"\n"
   "\n"
   "So when I was thinking a name for a messaging server, it amused me to think of it belting messages round a field with its tail!\n"
   "\n"
   "Of course another quote from Dante is most apt for anyone thinking of becoming involved in this kind of project…\n"
   "\n"
   "“Abandon Hope, all ye who enter here”…"
   ;

/*static*/bool TAboutBox::ShowAboutBox(QWidget *Owner,  bool onStartup )
{
   LogContainer->aboutBox = new TAboutBox( Owner, onStartup);


   int ret = LogContainer->aboutBox->exec();

   LogContainer->aboutBox = nullptr;

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

    trace(QString("TAboutbox geometry T %1 L %2 B %3 R %4").arg(geometry().top()).arg(geometry().left())
              .arg(geometry().bottom()).arg(geometry().right()));
    RegSettings settings;
    QByteArray ageometry = settings.getSettings().value("MinosAbout/geometry").toByteArray();
    if (ageometry.size() > 0)
        restoreGeometry(ageometry);
    trace(QString("TAboutbox geometry T %1 L %2 B %3 R %4").arg(geometry().top()).arg(geometry().left())
              .arg(geometry().bottom()).arg(geometry().right()));

    ui->PageControl1->setCurrentWidget(ui->AboutTabSheet);


    QString trmess = tr("TranslatorMessage");
    if (trmess == "TranslatorMessage")
    {
        trmess = "";
    }
    QString welcomeText = QString("<h2>%1<br><h1>%2 ") + STRINGVERSION  + " " + PRERELEASETYPE + " " + SecondInstall::getSecondInstallText() + "</h1>"
                                   "<br>%3"
                                   "<br><a href=\"http://minos.sourceforge.net/\">http://minos.sourceforge.net</a>"
                                   "<br><br>%4"
                                   "<br><a href=\"https://minos.groups.io/g/users\">https://minos.groups.io/g/users</a>"
                                   "<br><br><h2><a href=\"file://Manual\">%5</a>"
                                   "<br><br>";
    ui->AboutMemo->setText(welcomeText.arg(
                                            trmess
                                            , tr("Welcome to Minos Version")
                                            , tr("Home page")
                                            , tr("User forum (please join!)")
                                            , tr("Click here to read the manual!")
                                            ));
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
    ui->fontButton->setVisible(onStartup);
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

    showAppConfig();

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
    trace(QString("TAboutbox geometry T %1 L %2 B %3 R %4").arg(geometry().top()).arg(geometry().left())
              .arg(geometry().bottom()).arg(geometry().right()));

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
void TAboutBox::showAppConfig()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    ui->appsLabel->setText( minosConfig->getCurrConfig().configName);

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
    showAppConfig();
}
void TAboutBox::on_appSelectButton_clicked()
{
    StartConfigManager manageApps( this, true);   // when managing sets, include autostart
    manageApps.exec();
    showAppConfig();
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


void TAboutBox::on_fontButton_clicked()
{
    QString qpa = qgetenv("QT_QPA_PLATFORMTHEME");
    if (qpa.compare("qt5ct", Qt::CaseInsensitive) == 0)
    {
        mShowMessage(tr("Font setting will not work while the QT_QPA_PLATFORMTHEME environment variable is set to qt5ct"), this);
        RegSettings settings;
        settings.getSettings().remove( "font");
        return;
    }
    bool ok;
    QFont f;
    QFont nf = QFontDialog::getFont( &ok, f );
    if (ok && nf != f)
    {
        QApplication::setFont( nf );

        // for ( auto &widget: QApplication::allWidgets() )
        // {
        //     widget->setFont(nf);
        //     widget->update();
        // }

        RegSettings settings;
        settings.getSettings().setValue( "font", font() );
        appStart.emitFontChanged();

        MinosLoggerEvents::SendFontChanged();
    }
}

void TAboutBox::doNewContest(bool hf)
{
    LogContainer->setCurrSessionName(ui->chooseSetCb->currentText());

    QStringList names = LogContainer->FileNewActionExecute(hf, true);

    trace(QString("names %1").arg(names.join("!")));
    if (names.count() >= 3)
    {
        LogContainer->aboutBoxOpenFilename = names[0];

        QString newContestSetName = names[1];
        QString newContestAppSetName = names[2];
        // switch contest set and start apps to those of new contest
        //save contest set/contest file to be opened after preload

        if (!newContestSetName.isEmpty())
        {
            if (!ui->chooseSetCb->findText(newContestSetName))
            {
                // contestDetails may have added the set name
                ui->chooseSetCb->addItem(newContestSetName);
            }
            ui->chooseSetCb->setCurrentText(newContestSetName);
            trace(QString("TAboutBox::doNewContest set session name to %1").arg(newContestSetName));
            trace(QString("TAboutBox::doNewContest combo is %1").arg(ui->chooseSetCb->currentText()));
            repaint();
            trace(QString("TAboutBox::doNewContest combo after repaint is %1").arg(ui->chooseSetCb->currentText()));
        }

        if (!newContestAppSetName.isEmpty())
        {
            MinosConfig *minosConfig = MinosConfig::getMinosConfig();
            minosConfig->setCurConfig(newContestAppSetName);
            minosConfig->saveAll();
            showAppConfig();
        }
        accept();
    }
}
void TAboutBox::on_newHFButton_clicked()
{
    trace(QString("%1 entered").arg(__func__));
    doNewContest(true);
}


void TAboutBox::on_newVHFButton_clicked()
{
    trace(QString("%1 entered").arg(__func__));
    doNewContest(false);
}

