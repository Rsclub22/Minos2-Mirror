#include "base_pch.h"
#include "ContestApp.h"
#include "WsjtxMessageServer.hpp"
#include "WsjtxServer.h"

#include "WsjtxConfigure.h"
#include "ui_WsjtxConfigure.h"

using port_type = MessageServer::port_type;


WsjtxConfigure::WsjtxConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WsjtxConfigure)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QSettings settings;
    QByteArray geometry = settings.value("WsjtxConfigure/geometry/").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    ui->portSpinBox->setMinimum (1);
    ui->portSpinBox->setMaximum (std::numeric_limits<port_type>::max ());

    ui->portSpinBox_2->setMinimum (1);
    ui->portSpinBox_2->setMaximum (std::numeric_limits<port_type>::max ());

    ui->portSpinBox_3->setMinimum (1);
    ui->portSpinBox_3->setMaximum (std::numeric_limits<port_type>::max ());

    ui->portSpinBox_4->setMinimum (1);
    ui->portSpinBox_4->setMaximum (std::numeric_limits<port_type>::max ());

    {
        bool enabled;
        int port = 0;
        QString addr;

        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX1Enabled, enabled );
        TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTX1Port, port );
        TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX1GroupAddress, addr );

        ui->enabledcb->setChecked(enabled);
        ui->portSpinBox->setValue(port);
        ui->groupAddrEdit->setText(addr);

        ui->wsjtxPath->setText(WsjtxServer::getDataPath());
    }
    {
        bool enabled;
        int port = 0;
        QString addr;

        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX2Enabled, enabled );
        TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTX2Port, port );
        TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX2GroupAddress, addr );

        ui->enabledcb_2->setChecked(enabled);
        ui->portSpinBox_2->setValue(port);
        ui->groupAddrEdit_2->setText(addr);
    }
    {
        bool enabled;
        int port = 0;
        QString addr;

        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX3Enabled, enabled );
        TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTX3Port, port );
        TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX3GroupAddress, addr );

        ui->enabledcb_3->setChecked(enabled);
        ui->portSpinBox_3->setValue(port);
        ui->groupAddrEdit_3->setText(addr);
    }
    {
        bool enabled;
        int port = 0;
        QString addr;

        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX4Enabled, enabled );
        TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTX4Port, port );
        TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX4GroupAddress, addr );

        ui->enabledcb_4->setChecked(enabled);
        ui->portSpinBox_4->setValue(port);
        ui->groupAddrEdit_4->setText(addr);
    }
    ui->wsjtxPort->setValidator(new QIntValidator(0, 0xffff, this));
    bool wsjtxRbSelect;
    QString wsjtxRbAddr;
    int wsjtxRbPort;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpwsjtxRbSelect, wsjtxRbSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpwsjtxRbAddr, wsjtxRbAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpwsjtxRbPort, wsjtxRbPort );
    ui->wsjtxSelect->setChecked(wsjtxRbSelect);
    ui->wsjtxAddr->setText(wsjtxRbAddr);
    ui->wsjtxPort->setText(QString::number(wsjtxRbPort));


}

WsjtxConfigure::~WsjtxConfigure()
{
    delete ui;
}

void WsjtxConfigure::doClose()
{
    QSettings settings;
    settings.setValue("WsjtxConfigure/geometry/", saveGeometry());

    close();
}
void WsjtxConfigure::on_OKButton_clicked()
{
    {
        bool enabled = ui->enabledcb->isChecked();
        int port = ui->portSpinBox->value();
        QString addr = ui->groupAddrEdit->text();

        TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpWSJTX1Enabled, enabled );
        TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpWSJTX1Port, port );
        TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpWSJTX1GroupAddress, addr );
    }
    {
        bool enabled = ui->enabledcb_2->isChecked();
        int port = ui->portSpinBox_2->value();
        QString addr = ui->groupAddrEdit_2->text();

        TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpWSJTX2Enabled, enabled );
        TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpWSJTX2Port, port );
        TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpWSJTX2GroupAddress, addr );
    }
    {
        bool enabled = ui->enabledcb_3->isChecked();
        int port = ui->portSpinBox_3->value();
        QString addr = ui->groupAddrEdit_3->text();

        TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpWSJTX3Enabled, enabled );
        TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpWSJTX3Port, port );
        TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpWSJTX3GroupAddress, addr );
    }
    {
        bool enabled = ui->enabledcb_4->isChecked();
        int port = ui->portSpinBox_4->value();
        QString addr = ui->groupAddrEdit_4->text();

        TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpWSJTX4Enabled, enabled );
        TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpWSJTX4Port, port );
        TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpWSJTX4GroupAddress, addr );
    }

    bool wsjtxRbSelect = ui->wsjtxSelect->isChecked();
    QString wsjtxRbAddr = ui->wsjtxAddr->text();
    int wsjtxRbPort = ui->wsjtxPort->text().toInt();
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpwsjtxRbSelect, wsjtxRbSelect );
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpwsjtxRbAddr, wsjtxRbAddr );
    TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpwsjtxRbPort, wsjtxRbPort );

    WsjtxServer::getWsjtxServer()->start();

    doClose();
}

void WsjtxConfigure::on_CancelButton_clicked()
{
    doClose();
}

void WsjtxConfigure::on_alltxtBrowseButton_clicked()
{
    // search for WSJT-X data dir (which contains ALL.TXT)
    QDir cdir(GetCurrentDir());

    QString fpath =  ui->wsjtxPath->text();

    QString destDir = QFileDialog::getExistingDirectory(
                  this,
                  tr("Directory Containing ALL.TXT for WSJT-X"),
                  fpath,
                // NB Windows won't show files in a directory chooser
                  /*QFileDialog::ShowDirsOnly | */QFileDialog::DontResolveSymlinks
                   );
    if (!destDir.isEmpty())
    {
        QString rpath = cdir.relativeFilePath(destDir);
        ui->wsjtxPath->setText(rpath);
    }

}
