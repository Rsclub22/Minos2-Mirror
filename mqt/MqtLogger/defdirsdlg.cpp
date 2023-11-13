#include <QDir>
#include <QFileDialog>
#include "ContestApp.h"
#include "fileutils.h"
#include "tlogcontainer.h"
#include "waitcursor.h"

#include "defdirsdlg.h"
#include "ui_defdirsdlg.h"

DefDirsDlg::DefDirsDlg(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DefDirsDlg)
{
    ui->setupUi(this);

}

DefDirsDlg::~DefDirsDlg()
{
    delete ui;
}

void DefDirsDlg::initialise()
{
    QString deflog = TLogContainer::getDefaultDirectory(false);
    QString deflist = TLogContainer::getDefaultDirectory(true);

    ConfAge.initialise(&TContestApp::getContestApp() ->loggerBundle,elpAgeToProtectContests, ui->ageSpinner);
    ConfListDir.initialise(&TContestApp::getContestApp() ->loggerBundle,elpListDirectory, ui->listsDirEdit, deflist);
    ConfLogDir.initialise(&TContestApp::getContestApp() ->loggerBundle,elpLogDirectory, ui->logsDirEdit, deflog);

    ConfAgeProtectContests.initialise(&TContestApp::getContestApp() ->loggerBundle, elpAgeProtectContests, ui->ageCb);

    autoQRZCsHF.initialise(&TContestApp::getContestApp() ->loggerBundle,elpAutoQRZCsHF, ui->QRZCsHF);
    autoQRZCsVHF.initialise(&TContestApp::getContestApp() ->loggerBundle,elpAutoQRZCsVHF, ui->QRZCsVHF);
    autoQRZWSJTHF.initialise(&TContestApp::getContestApp() ->loggerBundle,elpAutoQRZWSJTHF, ui->QRZWSJTXHF);
    autoQRZWSJTVHF.initialise(&TContestApp::getContestApp() ->loggerBundle,elpAutoQRZWSJTVHF, ui->QRZWSJTVHF);

}
bool DefDirsDlg::check()
{
    return true;
}
void DefDirsDlg::cancel()
{

}
void DefDirsDlg::finalise()
{
    bool doSelectSession = false;

    ConfListDir.finalise();
    ConfLogDir.finalise();

    setDefLogDir(TLogContainer::getDefaultDirectory(false));
    setDefListDir(TLogContainer::getDefaultDirectory(true));

    if (ConfAgeProtectContests.finalise())
    {
        doSelectSession = true;
    }
    if (ConfAgeProtectContests.finalise())
    {
        doSelectSession = true;
    }
    autoQRZCsHF.finalise();
    autoQRZCsVHF.finalise();
    autoQRZWSJTHF.finalise();
    autoQRZWSJTVHF.finalise();

    if (doSelectSession)
    {
        TWaitCursor wc(this);
        LogContainer->selectSession(TContestApp::getContestApp()->currSession);
    }
}

QString DefDirsDlg::browseDefDir(const QString &prompt)
{
    QString c = GetCurrentDir();
    QDir cdir(GetCurrentDir());

    QString fpath =  ui->listsDirEdit->text();

    QString dest = QFileDialog::getExistingDirectory(
                  nullptr,
                  prompt,
                  fpath,
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                   );
    if (!dest.isEmpty())
    {
        // if path contains c then we should go relative, else stay absolute
        QDir destDir(dest);
        QString absDest = destDir.absolutePath();

        QString path = absDest;

        if (absDest.startsWith(c))
        {
            path = cdir.relativeFilePath(path);
        }

        return path;
    }
    return QString();
}

void DefDirsDlg::on_logsDirBrowse_clicked()
{

    QString res = browseDefDir(tr("Set Default Logs Directory"));
    if (!res.isEmpty())
    {
        ui->logsDirEdit->setText(res);
    }
}

void DefDirsDlg::on_listsDirBrowse_clicked()
{
    QString res = browseDefDir(tr("Set Default Lists Directory"));
    if (!res.isEmpty())
    {
        ui->listsDirEdit->setText(res);
    }
}

void DefDirsDlg::on_ageCb_stateChanged(int /*arg1*/)
{
    ui->ageSpinner->setEnabled(ui->ageCb->isChecked());
}

