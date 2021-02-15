#include "base_pch.h"

#include "tlogcontainer.h"
#include "Clusterbandmapconfigure.h"
#include "defdirsdlg.h"
#include "radiosettingdialog.h"
#include "WsjtxConfigure.h"
#include "n1mmbroadcastconfig.h"

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
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
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
    // the radiosettings pre and post-amble should go to the RadioSettingDialog class

    ClusterBandmapConfigure *cbc = new ClusterBandmapConfigure();
    DefDirsDlg *ddd = new DefDirsDlg();
    RadioSettingDialog *rdc = new RadioSettingDialog ();
    N1MMBroadcastConfig *nbc= new N1MMBroadcastConfig();
    WsjtxConfigure *wc = new WsjtxConfigure();

    cbc->initialise();
    ddd->initialise();
    rdc->initialise();
    nbc->initialise();
    wc->initialise();

    ui->optionTabs->addTab(cbc, tr("Cluster/Bandmap"));
    ui->optionTabs->addTab(ddd, tr("Default Directories"));
    ui->optionTabs->addTab(rdc, tr("Log Radio Settings"));
    ui->optionTabs->addTab(nbc, tr("UDP Broadcast"));
    ui->optionTabs->addTab(wc, tr("WSJT-X"));


    int ret = QDialog::exec();

    if (ret == QDialog::Accepted)
    {
        cbc->finalise();
        ddd->finalise();
        rdc->finalise();
        nbc->finalise();
        wc->finalise();

    }
    return ret;

}
void OptionsDialog::on_OKButton_clicked()
{
    accept();
}

void OptionsDialog::on_cancelButton_clicked()
{
    reject();
}
