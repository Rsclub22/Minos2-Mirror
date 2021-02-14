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

}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

int OptionsDialog::exec()
{
    // the radiosettings pre and post-amble should go to the RadioSettingDialog class

    QVector<QSharedPointer<BandInfo> > bands;
    BandList::getBandList().loadAllBands(bands);
    bool hfFlag = true;
    QSharedPointer<RadioSettingsDialogChangeFlag> radioSettingsDialogChangeFlag = QSharedPointer<RadioSettingsDialogChangeFlag>(new RadioSettingsDialogChangeFlag()) ;

    ui->optionTabs->addTab(new ClusterBandmapConfigure(), tr("Cluster/Bandmap"));
    ui->optionTabs->addTab(new DefDirsDlg(), tr("Default Directories"));
    ui->optionTabs->addTab(new RadioSettingDialog (hfFlag, bands, radioSettingsDialogChangeFlag, nullptr), tr("Log Radio Settings"));
    ui->optionTabs->addTab(new N1MMBroadcastConfig(), tr("UDP Broadcast"));
    ui->optionTabs->addTab(new WsjtxConfigure(), tr("WSJT-X"));


    int ret = QDialog::exec();

    if (radioSettingsDialogChangeFlag->isChanged())
    {
        if (radioSettingsDialogChangeFlag->serialComport)
        {
            QString comport = readSerialComportBandSwitchFromIni();
            if (!comport.isEmpty())
            {
                trace(QString("Bandswitch comport changed to %1").arg(comport));
                if (LogContainer->serialTVSw->getOpenFlag())
                {
                    trace(QString("Bandswitch comport open - closing"));
                    LogContainer->serialTVSw->closeComport();
                }

                if (LogContainer->serialTVSw->openComport(comport))
                {
                    trace(QString("Bandswitch comport %1 opened OK").arg(comport));

                }
                else
                {
                    QString errMsg = LogContainer->serialTVSw->error();
                    trace(QString("Bandswitch Comport failed to open = %1 Error = %2").arg(comport).arg(errMsg));
                }
            }
            else
            {
                trace(QString("Bandswitch comport changed, but comport is empty!"));
            }
        }

        emit LogContainer->logRadioSettingsChanged(radioSettingsDialogChangeFlag);
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
