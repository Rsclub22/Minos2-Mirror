#include <QFileInfo>
#include <QFileDialog>
#include <QFileSystemWatcher>

#include "MShowMessageDlg.h"
#include "MTrace.h"
#include "MinosParameters.h"
#include "MinosLoggerEvents.h"
#include "ContestApp.h"
#include "AppStartup.h"
#include "LoggerContest.h"
#include "AdifImport.h"
#include "contest.h"
#include "fileutils.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"

#include "adifmanager.h"
#include "manageadifdialog.h"
#include "ui_manageadifdialog.h"

// at contest open we need to get filename and lastOffset from .minos
// and set up a manager if required
// This allows for new records during Minos off time


//============================================================
ManageAdifDialog::ManageAdifDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManageAdifDialog)
{
    ui->setupUi(this);
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    lt = dynamic_cast<LoggerContestLog *>(ct);

    if (lt->adifManager)
    {
        ui->fileNameEdit->setText(lt->adifManager->getWatchedFile());
        ui->appendButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->monitorButton->setEnabled(false);
        ui->fileNameBrowse->setEnabled(false);
        ui->fileNameEdit->setEnabled(false);
    }
    else
    {
        ui->appendButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        ui->monitorButton->setEnabled(true);
        ui->fileNameBrowse->setEnabled(true);
        ui->fileNameEdit->setEnabled(true);
    }
}

ManageAdifDialog::~ManageAdifDialog()
{
    delete ui;
}

void ManageAdifDialog::on_OKButton_clicked()
{
    accept();
}

void ManageAdifDialog::on_fileNameBrowse_clicked()
{
    QString InitialDir = getDirectoryLocation(dlLogs);

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = tr("ADIF files (*.adi);;"
                        "All Files (*.*)") ;

    QString fname = QFileDialog::getOpenFileName( this,
                                                 tr("Open ADIF file"),
                                                 InitialDir,  // dir
                                                 Filter
                                                 );

    if (!fname.isEmpty())
    {
        ui->fileNameEdit->setText(fname);
    }

}

void ManageAdifDialog::on_monitorButton_clicked()
{
    // How should we indicate that this is live? Add "monitor ADIF" suffix
    // to the tab name?

    // Is it tied to a particular contest (yes); need monitored file and seek point
    // in .minos

    // we need to optionally append (so ADIF doesn't have to be initially empty),
    // and then keep monitoring the file, appending as and when there are changes
    // we do this line by line, using ADIFImport::doImportADIFString

    // Do we hold the file open (read only)?

    // we get a signal when the file changes, so we keep the read offset in the contest
    // If we move machines, then we have to allow for the linked file not existing, or
    // beeing a different size so the seek point doesn't exist

    if (lt->adifManager)
    {
        // override or cancel?
        lt->adifManager.reset();
    }

    QString fname = ui->fileNameEdit->text().trimmed();
    if (fname.size())
    {
        BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
        LoggerContestLog * lt = dynamic_cast<LoggerContestLog *>(ct);
        if (lt)
        {
            // -1 says look at file size
            // otherwise we need to get from contest
            qint64 lo = -1;
            if (ui->appendMonitorCB->isChecked())
            {
                lo = 0;
            }
            lt->adifManager = QSharedPointer<AdifManager>(new AdifManager(lt, fname, lo));
            ui->appendButton->setEnabled(false);
            ui->stopButton->setEnabled(true);
            ui->monitorButton->setEnabled(false);
            ui->fileNameBrowse->setEnabled(false);
            ui->fileNameEdit->setEnabled(false);
        }
        else
        {
            mShowMessage(tr("No current contest"), this);
        }
    }
    else
    {
        ui->fileNameEdit->setFocus();
    }
}
void ManageAdifDialog::on_stopButton_clicked()
{
    if (lt->adifManager)
    {
        // override or cancel?
        lt->adifManager.reset();
    }
    ui->appendButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->monitorButton->setEnabled(true);
    ui->fileNameBrowse->setEnabled(true);
    ui->fileNameEdit->setEnabled(true);
}


void ManageAdifDialog::on_appendButton_clicked()
{
    QString fname = ui->fileNameEdit->text();

    if (!fname.isEmpty())
    {
        QIODevice::OpenMode om = QIODevice::ReadOnly;

        QSharedPointer<QFile> adifFile(new QFile(fname));

        if (!adifFile->open(om))
        {
            QString lerr = adifFile->errorString();
            QString emess = tr("Failed to open ADIF file %1 : %2").arg(fname, lerr);
            MinosParameters::getMinosParameters() ->mshowMessage( emess );
            return;
        }

        trace(QString("Appending ADIF log %1 to %2").arg(fname, lt->cfileName));
        int spoint = lt->ctList.count();
        if (! ADIFImport::doImportADIFLog(dynamic_cast<LoggerContestLog *>(lt),  adifFile ))
        {
            MinosParameters::getMinosParameters() ->mshowMessage( tr("Failed to append %1").arg(fname) );
        }
        for ( int i = spoint; i != lt->ctList.count(); i++ )
        {
            QSharedPointer<BaseContact> bct = lt->pcontactAt(i);
            bct->commonSave(bct);
        }
        lt->commonSave( false );
        lt->scanContest();          // after append ADIF file, required
        //ct->validateLoc();
        for ( int i = spoint; i != lt->ctList.count(); i++ )
        {
            QSharedPointer<BaseContact> bct = lt->pcontactAt(i);
            MinosLoggerEvents::SendAfterLogContact(lt, bct);          // after append ADIF file
        }
        TSingleLogFrame * tslf = LogContainer ->findContest( lt );

        tslf->updateTrees();
        tslf->startNextEntry();   //(AppendAdifActionExecute())
    }
}
