#include <QFileDialog>

#include "kstconfigure.h"
#include "AppStartup.h"
#include "MShowMessageDlg.h"
#include "fileutils.h"
#include "soundplayer.h"

#include "ui_kstconfigure.h"


KSTConfigure::KSTConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KSTConfigure)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->portFrame->setVisible(true);
    ui->serverFrame->setVisible(true);

    ui->callsignEdit->setValidator(&userValidator);
    ui->locatorEdit->setValidator(&locValidator);

}
int KSTConfigure::exec()
{
    ui->portEdit->setValidator(new QIntValidator(0, 0xffff, this));

    ui->serverEdit->setText(hostname);
    ui->portEdit->setText(port);
    ui->callsignEdit->setText(username);
    ui->passwordEdit->setText(password);

    ui->locatorEdit->setText(locator);
    ui->nameEdit->setText(firstName);
    ui->autoConnect->setChecked(autoConnect);

    ui->meepSound->setText(meepSoundFile);
    ui->meepNotifyLogger->setChecked(meepNotifyLogger);
    ui->meepPlaySound->setChecked(meepPlaySound);
    ui->volumeSpinBox->setValue(meepVolume);

    ui->ASServerName->setText(ASServerName);
    ui->ASMyName->setText(ASMyName);

    ui->ASMaxDistance->setText(QString::number(ASMaxDistance));
    ui->ASMaxDistance->setValidator(new QIntValidator(0, 0xffff, this));
    ui->ASMinDistance->setText(QString::number(ASMinDistance));
    ui->ASMinDistance->setValidator(new QIntValidator(0, 0xffff, this));
    ui->ASPort->setText(QString::number( ASPort));
    ui->ASPort->setValidator(new QIntValidator(0, 0xffff, this));
    ui->ASTimeout->setText(QString::number( ASTimeout));
    ui->ASTimeout->setValidator(new QIntValidator(1, 60, this));

    return QDialog::exec();
}
KSTConfigure::~KSTConfigure()
{
    delete ui;
}

void KSTConfigure::on_cancelButton_clicked()
{
    reject();
}

void KSTConfigure::on_OKButton_clicked()
{
    hostname = ui->serverEdit->text();
    port = ui->portEdit->text();
    username = ui->callsignEdit->text();
    password = ui->passwordEdit->text();
    autoConnect = ui->autoConnect->isChecked();
    locator = ui->locatorEdit->text().toUpper();
    firstName = ui->nameEdit->text();
    if (firstName.length() >16)
    {
        mShowMessage(tr("Name %1 cannot be more than 16 characters").arg(firstName), this);
        return;
    }

    meepSoundFile = ui->meepSound->text().trimmed();
    meepPlaySound = ui->meepPlaySound->isChecked();
    meepNotifyLogger = ui->meepNotifyLogger->isChecked();

    ASServerName = ui->ASServerName->text();
    ASMyName = ui->ASMyName->text();

    ASMaxDistance = ui->ASMaxDistance->text().toInt();
    ASMinDistance = ui->ASMinDistance->text().toInt();

    ASPort = ui->ASPort->text().toInt();
    ASTimeout = ui->ASTimeout->text().toInt();

    if (hostname.isEmpty() || port.isEmpty() ||username.isEmpty() ||password.isEmpty())
        return;

    accept();
}

void KSTConfigure::on_meepBrowse_clicked()
{
    QString InitialDir = meepSoundFile;

    if (meepSoundFile.isEmpty())
    {
        InitialDir = getDirectoryLocation(dlConfiguration);

        QFileInfo qf(InitialDir);

        InitialDir = qf.canonicalFilePath();
    }
    QString Filter = tr("WAV (*.wav);;"
                        "All Files (*.*)") ;

    QString fname = QFileDialog::getOpenFileName( this,
                                                 tr("Meep Notification Sound"),
                                                 InitialDir,  // dir
                                                 Filter
                                                 );

    if (!fname.isEmpty())
    {
        meepSoundFile = fname;
        ui->meepSound->setText(fname);
    }
}

void KSTConfigure::on_testButton_clicked()
{
    QString fname = ui->meepSound->text();
    trace(QString("Test meep using %1").arg(fname));
    if (meepPlaySound && FileExists(fname))
    {
        SoundPlayer::playSound(fname, meepVolume);

    }
    else
    {
        trace(QString("%1 doesn't exist").arg(fname));

    }
}

void KSTConfigure::on_volumeSpinBox_valueChanged(int arg1)
{
    meepVolume = arg1;
}

