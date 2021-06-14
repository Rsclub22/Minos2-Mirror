#include "base_pch.h"

#include "tlogcontainer.h"
#include "Clusterbandmapconfigure.h"
#include "defdirsdlg.h"
#include "DisplayOptions.h"
#include "radiosettingdialog.h"
#include "WsjtxConfigure.h"
#include "n1mmbroadcastconfig.h"
#include "MinosLoggerEvents.h"

#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QSettings settings;
    QByteArray geometry = settings.value("OptionsDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MainRaised, this, &OptionsDialog::onMainRaised);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}
void OptionsDialog::onMainRaised()
{
    raise();
}
void OptionsDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("OptionsDialog/geometry", saveGeometry());
}
void OptionsDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void OptionsDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}

int OptionsDialog::exec()
{
    QSettings settings;
    int curTabNo = settings.value("OptionsDialog/curTab").toInt();

    cbc = new ClusterBandmapConfigure();
    ddd = new DefDirsDlg();
    dod = new DisplayOptions();
    rdc = new RadioSettingDialog ();
    nbc = new N1MMBroadcastConfig();
    wc  = new WsjtxConfigure();

    cbc->initialise();
    cbc->setAutoFillBackground(true);
    ddd->initialise();
    ddd->setAutoFillBackground(true);
    dod->initialise();
    dod->setAutoFillBackground(true);
    rdc->initialise();
    rdc->setAutoFillBackground(true);
    nbc->initialise();
    nbc->setAutoFillBackground(true);
    wc->initialise();
    wc->setAutoFillBackground(true);

    ui->optionTabs->addTab(cbc, tr("Cluster/Bandmap"));
    ui->optionTabs->addTab(ddd, tr("Default Directories"));
    ui->optionTabs->addTab(dod, tr("DisplayOptions"));
    ui->optionTabs->addTab(rdc, tr("Log Radio Settings"));
    ui->optionTabs->addTab(nbc, tr("UDP Broadcast"));
    ui->optionTabs->addTab(wc, tr("WSJT-X"));

    ui->optionTabs->setCurrentIndex(curTabNo);

    int ret = QDialog::exec();

    if (ret == QDialog::Accepted)
    {
        cbc->finalise();
        ddd->finalise();
        dod->finalise();
        rdc->finalise();
        nbc->finalise();
        wc->finalise();

    }
    return ret;

}
bool OptionsDialog::check()
{
    return cbc->check()
           && ddd->check()
           && dod->check()
           && rdc->check()
           && nbc->check()
           && wc->check() ;
}
void OptionsDialog::on_OKButton_clicked()
{
    if (check())
    {
        accept();
    }
}

void OptionsDialog::on_cancelButton_clicked()
{
    cbc->cancel();
    ddd->cancel();
    dod->cancel();
    rdc->cancel();
    nbc->cancel();
    wc->cancel();
    reject();
}

void OptionsDialog::on_optionTabs_currentChanged(int index)
{
    QSettings settings;
    settings.setValue("OptionsDialog/curTab", index);
}
