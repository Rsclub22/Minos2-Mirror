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

    bool enabled;
    int port = 0;
    QString addr;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTXEnabled, enabled );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTXPort, port );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTXGroupAddress, addr );

    ui->enabledcb->setChecked(enabled);
    ui->portSpinBox->setValue(port);
    ui->groupAddrEdit->setText(addr);

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
    bool enabled = ui->enabledcb->isChecked();
    int port = ui->portSpinBox->value();
    QString addr = ui->groupAddrEdit->text();

    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpWSJTXEnabled, enabled );
    TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpWSJTXPort, port );
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpWSJTXGroupAddress, addr );

    WsjtxServer::getWsjtxServer()->start();

    doClose();
}

void WsjtxConfigure::on_CancelButton_clicked()
{
    doClose();
}
