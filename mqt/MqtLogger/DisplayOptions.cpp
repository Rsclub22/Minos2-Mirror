#include <QFontDialog>
#include "base_pch.h"
#include "ContestApp.h"
#include "MinosLoggerEvents.h"
#include "tlogcontainer.h"
#include "ConfigFile.h"
#include "DisplayOptions.h"
#include "ui_DisplayOptions.h"

DisplayOptions::DisplayOptions(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DisplayOptions)
{
    ui->setupUi(this);

}

DisplayOptions::~DisplayOptions()
{
    delete ui;
}

void DisplayOptions::initialise()
{
    so = LogContainer->isShowOperators();
    ui->ShowOperatorscb->setChecked(so);

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAutoFill, autoFill );
    ui->ReportAutofillcb->setChecked(autoFill);

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpTabforSandP, TabSandP );
    ui->TabSandPActioncb->setChecked(TabSandP);

    TContestApp::getContestApp() ->loggerBundle.getIntProfile(elpAgeToProtectContests, cap);
    ui->ageSpinner->setValue(cap);

    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    ui->ListCompressionSpinner->setValue(lcf);

    TContestApp::getContestApp() ->getIntDisplayProfile(edpQSOFieldFont, qff);
    ui->QSOFieldExpansionSpinner->setValue(qff);

    f = font();
    //ui->FontLabel->setText(f.toString());

    QString currentLang = getCurrentLanguage();

    QVector<Translation> languages = getLanguages();
    QStringList sl;

    int n = 0;
    for(auto const &l: languages)
    {
        sl.append(l.dispName);
        if (l.code == currentLang)
        {
            currLang = n;
        }
        n++;
    }
    ui->LanguageComboBox->addItems(sl);
    ui->LanguageComboBox->setCurrentIndex(currLang);
}

void DisplayOptions::finalise()
{
    bool doSelectSession = false;

    bool nso = ui->ShowOperatorscb->isChecked();
    if (nso != so)
    {
        TContestApp::getContestApp() ->displayBundle.setBoolProfile( edpShowOperators, nso );
        TContestApp::getContestApp() ->displayBundle.flushProfile();
        MinosLoggerEvents::SendShowOperators();
    }

    bool nautoFill = ui->ReportAutofillcb->isChecked();
    if (autoFill != nautoFill)
    {
        TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpAutoFill, nautoFill );
        TContestApp::getContestApp() ->loggerBundle.flushProfile();
    }

    bool nTabSandP = ui->TabSandPActioncb->isChecked();
    if (TabSandP != nTabSandP)
    {
        TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpTabforSandP, nTabSandP );
        TContestApp::getContestApp() ->loggerBundle.flushProfile();

        MinosLoggerEvents::SendTabSandP();
    }
    int ncap = ui->ageSpinner->value();
    if (cap != ncap)
    {
        TContestApp::getContestApp() ->loggerBundle.setIntProfile(elpAgeToProtectContests, ncap);
        TContestApp::getContestApp() ->loggerBundle.flushProfile();

        doSelectSession = true;
    }

    int nlcf = ui->ListCompressionSpinner->value();
    if (nlcf != lcf)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpListCompression, nlcf);
        MinosLoggerEvents::sendListCompressionChanged(nlcf/100.0);
        doSelectSession = true;
    }

    int nqff = ui->QSOFieldExpansionSpinner->value();
    if (qff != nqff)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpQSOFieldFont, qff);
        MinosLoggerEvents::SendFontChanged();
        doSelectSession = true;
    }
    if (f != nf)
    {
        doFontChange();
        doSelectSession = true;
    }

    int lang = ui->LanguageComboBox->currentIndex();
    if (lang >= 0 && lang != currLang)
    {
        doLanguageChange();
        doSelectSession = true;
    }

    if (doBounceOnExit)
    {
        TWaitCursor wc(this);
        MinosConfig::getMinosConfig() ->bounce();
    }

    if (doSelectSession)
    {
        TWaitCursor wc(this);
        LogContainer->selectSession(TContestApp::getContestApp()->currSession);
    }
}

void DisplayOptions::on_FontChangeButton_clicked()
{
    QString qpa = qgetenv("QT_QPA_PLATFORMTHEME");
    if (qpa.compare("qt5ct", Qt::CaseInsensitive) == 0)
    {
        mShowMessage(tr("Font setting will not work while the QT_QPA_PLATFORMTHEME environment variable is set to qt5ct"), this);
        QSettings settings;
        settings.remove( "font");
        return;
    }
    bool ok;
    nf = QFontDialog::getFont( &ok, f );
    if (ok)
    {
        //ui->FontLabel->setText(nf.toString());
        ui->FontLabel->setFont(nf);
        ui->FontLabel->update();
    }

}

void DisplayOptions::doFontChange()
{
    if (nf != f)
    {
        bool serverRunning = checkServerReady();
        QApplication::setFont( nf );

        for ( auto const &widget: QApplication::allWidgets() )
        {
            widget->setFont(nf);
            widget->update();
        }

        QSettings settings;
        settings.setValue( "font", font() );

        MinosLoggerEvents::SendFontChanged();
        if (serverRunning)
        {
            doBounceOnExit = true;
        }
        f = nf;
    }
}

void DisplayOptions::doLanguageChange()
{
    int lang = ui->LanguageComboBox->currentIndex();
    if (lang >= 0 && lang != currLang)
    {
        bool serverRunning = checkServerReady();

        QString selText = ui->LanguageComboBox->currentText();

        QVector<Translation> languages = getLanguages();
        for(auto const &l: languages)
        {
            if (l.dispName == selText)
            {
                // this propagates a changeEvent that causes all contests to reload
                switchTranslation(l.code);
                break;
            }
        }
        if (serverRunning)
        {
            doBounceOnExit = true;
        }
    }
}
