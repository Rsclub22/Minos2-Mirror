#include "base_pch.h"
#include "tlogcontainer.h"
#include "defdirsdlg.h"
#include "ui_defdirsdlg.h"

DefDirsDlg::DefDirsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DefDirsDlg)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QSettings settings;
    QByteArray geometry = settings.value("DefDirsDlg/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QString deflog = TLogContainer::getDefaultDirectory(false);
    ui->logsDirEdit->setText(deflog);
    QString deflist =TLogContainer::getDefaultDirectory(true);
    ui->listsDirEdit->setText(deflist);
}

DefDirsDlg::~DefDirsDlg()
{
    delete ui;
}
void DefDirsDlg::doCloseEvent()
{
    QSettings settings;
    settings.setValue("DefDirsDlg/geometry", saveGeometry());
}
void DefDirsDlg::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void DefDirsDlg::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void DefDirsDlg::on_OKButton_clicked()
{
    accept();
}

void DefDirsDlg::on_cancelButton_clicked()
{
    reject();
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
