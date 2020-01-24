#include "base_pch.h"
#include <QDesktopServices>

#include "StartConfig.h"
#include "ConfigFile.h"
#include "taboutbox.h"
#include "ui_taboutbox.h"

static bool started = false;

QString TAboutBox::welcomeText = QString("<br><h1>%1 ") + STRINGVERSION  + " " + PRERELEASETYPE + "</h1>"
                               "<br><a href=\"http://minos.sourceforge.net/\">http://minos.sourceforge.net</a>"
                               "<br><br>";

const char * TAboutBox::wpart = QT_TR_NOOP("Welcome to Minos Version");

QString TAboutBox::creditsText = QString("<br><h2>%1</h2>"
                              "<br>Mike Goodey G0GJV; David Balharrie M0DGB/G8FKH; Neil Yorke M0NKE"
                              "<br>Dave Sergeant G3YMC; Peter Burton G3ZPB, Ken Punshon G4APJ"
                              "<br><br>");
const char *TAboutBox::cpart = QT_TR_NOOP("Credits");
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
    if ( !started && doStartup )
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

    QSettings settings;
    QByteArray geometry = settings.value("MinosAbout/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->PageControl1->setCurrentWidget(ui->AboutTabSheet);

    ui->AboutMemo->setText(welcomeText.arg(tr(wpart)));
    ui->AboutMemo->setTextFormat(Qt::RichText);
    ui->AboutMemo->setTextInteractionFlags(Qt::TextBrowserInteraction);

    ui->CreditsMemo->setTextFormat(Qt::RichText);
    ui->CreditsMemo->setText(creditsText.arg(cpart));

    ui->MinosMemo->setText(MinosText);

    ui->ExitButton->setVisible(onStartup);
    ui->LoggerOnlyButton->setVisible(onStartup);

    if (  onStartup && !checkServerReady() )
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
    QSettings settings;
    settings.setValue("MinosAbout/geometry", saveGeometry());
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
    QDesktopServices::openUrl(QUrl(link));
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
    StartConfig configBox( this, false);
    configBox.exec();
}
