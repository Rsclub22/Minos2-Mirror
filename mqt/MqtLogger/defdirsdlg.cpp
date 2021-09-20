#include "base_pch.h"
#include "ContestApp.h"
#include "tlogcontainer.h"
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
    ui->logsDirEdit->setText(deflog);
    QString deflist =TLogContainer::getDefaultDirectory(true);
    ui->listsDirEdit->setText(deflist);

    TContestApp::getContestApp() ->loggerBundle.getIntProfile(elpAgeToProtectContests, cap);
    ui->ageSpinner->setValue(cap);

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAgeProtectContests, doAge );
    ui->ageCb->setChecked(doAge);
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
    QString temp;

    temp = ui->logsDirEdit->text().trimmed();
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpLogDirectory, temp );


    temp = ui->listsDirEdit->text().trimmed();
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpListDirectory, temp );

    int ncap = ui->ageSpinner->value();
    if (cap != ncap)
    {
        TContestApp::getContestApp() ->loggerBundle.setIntProfile(elpAgeToProtectContests, ncap);
        TContestApp::getContestApp() ->loggerBundle.flushProfile();

        doSelectSession = true;
    }
    bool ndoAge = ui->ageCb->isChecked();
    if (doAge != ndoAge)
    {
        TContestApp::getContestApp() ->loggerBundle.setBoolProfile(elpAgeToProtectContests, ndoAge);
        TContestApp::getContestApp() ->loggerBundle.flushProfile();

        doSelectSession = true;
    }

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

