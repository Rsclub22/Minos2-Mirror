#include <QFontDialog>
#include "regsettings.h"
#include "AppStartup.h"
#include "ContestApp.h"
#include "MShowMessageDlg.h"
#include "MinosLoggerEvents.h"
#include "tlogcontainer.h"
#include "ConfigFile.h"
#include "waitcursor.h"
#include "ServerEvent.h"

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
    ShowOperators.initialise(&TContestApp::getContestApp() ->displayBundle, edpShowOperators, ui->ShowOperatorscb);
    ReadabilityInit.initialise(&TContestApp::getContestApp() ->loggerBundle, elpReadabilityInit, ui->ReadabilityAutofillcb);
    AutoFill.initialise(&TContestApp::getContestApp() ->loggerBundle, elpAutoFill, ui->ReportAutofillcb);
    TabforSandP.initialise(&TContestApp::getContestApp() ->loggerBundle, elpTabforSandP, ui->TabSandPActioncb);
    SeparateIcons.initialise(&TContestApp::getContestApp() ->displayBundle, edpSeparateIcons, ui->separateIconsCb);
    ExpertMode.initialise(&TContestApp::getContestApp() ->displayBundle, edpExpertMode, ui->expertModeCheckBox);
    AlternateFKeys.initialise(&TContestApp::getContestApp() ->displayBundle, edpAlternateFKeys, ui->alternateFKeysCheckBox);
    ShowAuxHeaders.initialise(&TContestApp::getContestApp() ->loggerBundle, elpShowAuxHeaders, ui->ShowAuxHeadersCheckBox);

    ShowSingleBandInCrib.initialise(&TContestApp::getContestApp() ->loggerBundle, elpShowCribBand, ui->nextContactBandcb);
    ShowQSOMapGrid.initialise(&TContestApp::getContestApp() ->loggerBundle, elpShowQSOMapGrid, ui->QSOMapShowGrid);
    ShowQSOMapLines.initialise(&TContestApp::getContestApp() ->loggerBundle, elpShowQSOMapLines, ui->QSOMapShowLines);

    MapShowCluster.initialise(&TContestApp::getContestApp() ->loggerBundle, elpMapShowCluster, ui->mapShowCluster);
    MapClusterDistance.initialise(&TContestApp::getContestApp() ->loggerBundle, elpMapClusterDistance, ui->mapClusterDistance);

#ifndef Q_OS_WIN
    ui->separateIconsCb->hide();
#endif

    int temp;
    TContestApp::getContestApp() ->loggerBundle.getIntProfile(elpShowOperateTime, temp);
    sot = static_cast<SHOWOPERATINGTIME>(temp);
    switch(sot)
    {
    default:
    case otNone:
        ui->otNonerb->setChecked(true);
        break;
    case otRSGB:
        ui->otRSGBrb->setChecked(true);
        break;
    case otIARU:
        ui->otIARUrb->setChecked(true);
        break;
    }
    TContestApp::getContestApp() ->loggerBundle.getIntProfile(elpLocMapCentre, temp);
    lmc = static_cast<LOCMAPCENTRE>(temp);
    switch(lmc)
    {
    default:
    case lmsDontMove:
        ui->locDontMoverb->setChecked(true);
        break;

    case lmsMyLoc:
        ui->locMyLocrb->setChecked(true);
        break;

    case lmsClicked:
        ui->locLastLocrb->setChecked(true);
        break;

    case lmsCentre:
        ui->locCentrerb->setChecked(true);
        break;
    }


    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    ui->ListCompressionSpinner->setValue(lcf);

    TContestApp::getContestApp() ->getIntDisplayProfile(edpQSOFieldFont, qff);
    ui->QSOFieldExpansionSpinner->setValue(qff);

    f = font();
    nf = f;
    //ui->FontLabel->setText(f.toString());

    QString currentLang = getCurrentLanguage();

    QVector<Translation> languages = getLanguages();
    QStringList sl;

    int n = 0;
    for(auto const &l: qAsConst(languages))
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


    ui->ls->setValue(ls);

    TContestApp::getContestApp() ->getIntDisplayProfile(edpls, ls);
    ui->ls->setValue(ls);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcml, cml);
    ui->cml->setValue(cml);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmt, cmt);
    ui->cmt->setValue(cmt);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmr, cmr);
    ui->cmr->setValue(cmr);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmb, cmb);
    ui->cmb->setValue(cmb);

}
bool DisplayOptions::check()
{
    return true;
}
void DisplayOptions::cancel()
{

}
void DisplayOptions::finalise()
{
    bool doSelectSession = false;

    if (ShowOperators.finalise())
    {
        MinosLoggerEvents::SendShowOperators();
    }
    ReadabilityInit.finalise();
    AutoFill.finalise();
    if (TabforSandP.finalise())
    {
        MinosLoggerEvents::SendTabSandP();
    }
    if (SeparateIcons.finalise())
    {
        doSelectSession = true;
    }
    if (AlternateFKeys.finalise())
    {
        doSelectSession = true;
    }
    if (ExpertMode.finalise())
    {
        doSelectSession = true;
    }

    if (ShowAuxHeaders.finalise())
    {
        MinosLoggerEvents::SendShowAuxHeaders();
    }
    if (ShowSingleBandInCrib.finalise())
    {
        MinosLoggerEvents::SendShowCribBand();
    }
    bool gchanged = ShowQSOMapGrid.finalise();
    bool lchanged = ShowQSOMapLines.finalise();

    bool mscchanged = MapShowCluster.finalise();
    bool mcdchanged = MapClusterDistance.finalise();


    if (gchanged || lchanged || mscchanged || mcdchanged)
    {
        MinosLoggerEvents::SendRedrawQSOMap(ShowQSOMapGrid.value(),
                                            ShowQSOMapLines.value(),
                                            MapShowCluster.value(),
                                            MapClusterDistance.iValue()
                                            );
    }

    MinosLoggerEvents::SendShowAuxHeaders();
    SHOWOPERATINGTIME nsot = otNone;
    if (ui->otNonerb->isChecked())
    {
        nsot = otNone;
    }
    else if (ui->otRSGBrb->isChecked())
    {
        nsot = otRSGB;
    }
    else if (ui->otIARUrb->isChecked())
    {
        nsot = otIARU;
    }
    if (nsot != sot)
    {
        TContestApp::getContestApp() ->loggerBundle.setIntProfile(elpShowOperateTime, nsot);
        TContestApp::getContestApp() ->loggerBundle.flushProfile();
        sot = nsot;
    }

    LOCMAPCENTRE nlmc = lmsMyLoc;
    if (ui->locDontMoverb->isChecked())
    {
        nlmc = lmsDontMove;
    }
    else if (ui->locMyLocrb->isChecked())
    {
        nlmc = lmsMyLoc;
    }
    else if (ui->locLastLocrb->isChecked())
    {
        nlmc = lmsClicked;
    }
    else if (ui->locCentrerb->isChecked())
    {
        nlmc = lmsCentre;
    }
    if (nlmc != lmc)
    {
        TContestApp::getContestApp() ->loggerBundle.setIntProfile(elpLocMapCentre, nlmc);
        TContestApp::getContestApp() ->loggerBundle.flushProfile();
        lmc = nlmc;

        BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
        MinosLoggerEvents::SendRefreshStackMults(ct);

    }

    int nlcf = ui->ListCompressionSpinner->value();
    if (nlcf != lcf)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpListCompression, nlcf);
        TContestApp::getContestApp() ->displayBundle.flushProfile();
        MinosLoggerEvents::SendListCompressionChanged(nlcf/100.0);
        doSelectSession = true;
    }

    int nqff = ui->QSOFieldExpansionSpinner->value();
    if (qff != nqff)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpQSOFieldFont, nqff);
        TContestApp::getContestApp() ->displayBundle.flushProfile();
        MinosLoggerEvents::SendFontChanged();
        doSelectSession = true;
    }

    // f and nf are set by the font choise button
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

    bool sendMargins = false;
    int nls = ui->ls->value();
    if (nls != ls)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpls, nls);
        sendMargins = true;
    }
    int ncml = ui->cml->value();
    if (ncml != cml)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpcml, ncml);
        sendMargins = true;
    }
    int ncmt = ui->cmt->value();
    if (ncmt != cmt)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpcmt, ncmt);
        sendMargins = true;
    }
    int ncmr = ui->cmr->value();
    if (ncmr != cmr)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpcmr, ncmr);
        sendMargins = true;
    }
    int ncmb = ui->cmb->value();
    if (ncmb != cmb)
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpcmb, ncmb);
        sendMargins = true;
    }

    if (doBounceOnExit)
    {
        TWaitCursor wc(this);
        MinosConfig::getMinosConfig() ->bounce();
    }
    TContestApp::getContestApp() ->loggerBundle.flushProfile();
    TContestApp::getContestApp() ->displayBundle.flushProfile();

    if (doSelectSession)
    {
        TWaitCursor wc(this);
        LogContainer->selectSession(TContestApp::getContestApp()->currSession);
    }
    else if (sendMargins)
    {
        MinosLoggerEvents::SendQSOMargins();
    }
}

void DisplayOptions::on_FontChangeButton_clicked()
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
        bool routerRunning = checkRouterReady();
        QApplication::setFont( nf );

        for ( auto &widget: QApplication::allWidgets() )
        {
            widget->setFont(nf);
            widget->update();
        }

        RegSettings settings;
        settings.getSettings().setValue( "font", font() );

        MinosLoggerEvents::SendFontChanged();
        if (routerRunning)
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
        bool routerRunning = checkRouterReady();

        QString selText = ui->LanguageComboBox->currentText();

        QVector<Translation> languages = getLanguages();
        for(auto const &l: qAsConst(languages))
        {
            if (l.dispName == selText)
            {
                // this propagates a changeEvent that causes all contests to reload
                switchTranslation(l.code);
                break;
            }
        }
        if (routerRunning)
        {
            doBounceOnExit = true;
        }
    }
}
