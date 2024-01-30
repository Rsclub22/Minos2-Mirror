#include <QFileInfo>
#include <QFileDialog>

#include "MTrace.h"
#include "MinosParameters.h"
#include "ContestApp.h"
#include "AppStartup.h"
#include "LoggerContest.h"
#include "AdifImport.h"
#include "contest.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"

#include "manageadifdialog.h"
#include "ui_manageadifdialog.h"

ManageAdifDialog::ManageAdifDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManageAdifDialog)
{
    ui->setupUi(this);
}

ManageAdifDialog::~ManageAdifDialog()
{
    delete ui;
}

void ManageAdifDialog::on_OKButton_clicked()
{
    accept();
}


void ManageAdifDialog::on_cancelButton_clicked()
{
    reject();
}


void ManageAdifDialog::on_fileNameBrowse_clicked()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();

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

}


void ManageAdifDialog::on_appendButton_clicked()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();

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

        trace(QString("Appending ADIF log %1 to %2").arg(fname, ct->cfileName));
        int spoint = ct->ctList.count();
        if (! ADIFImport::doImportADIFLog(dynamic_cast<LoggerContestLog *>(ct),  adifFile ))
        {
            MinosParameters::getMinosParameters() ->mshowMessage( tr("Failed to append %1").arg(fname) );
        }
        for ( int i = spoint; i != ct->ctList.count(); i++ )
        {
            QSharedPointer<BaseContact> bct = ct->pcontactAt(i);
            bct->commonSave(bct);
        }
        ct->commonSave( false );
        ct->scanContest();          // after append ADIF file, required
        //ct->validateLoc();
        for ( int i = spoint; i != ct->ctList.count(); i++ )
        {
            QSharedPointer<BaseContact> bct = ct->pcontactAt(i);
            MinosLoggerEvents::SendAfterLogContact(ct, bct);          // after append ADIF file
        }
        TSingleLogFrame * tslf = LogContainer ->findContest( ct );

        tslf->updateTrees();
        tslf->startNextEntry();   //(AppendAdifActionExecute())
    }
}

