#include "base_pch.h"
#include "ContestApp.h"
#include "tlogcontainer.h"

#include "n1mmbroadcastconfig.h"
#include "ui_n1mmbroadcastconfig.h"

N1MMBroadcastConfig::N1MMBroadcastConfig(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::N1MMBroadcastConfig)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

}

N1MMBroadcastConfig::~N1MMBroadcastConfig()
{
    delete ui;
}

void N1MMBroadcastConfig::initialise()
{
    ui->LookupFrame->setVisible(false);

    ui->contactsPort->setValidator(new QIntValidator(0, 0xffff, this));
    ui->extCSPort->setValidator(new QIntValidator(0, 0xffff, this));
    ui->ADIFPort->setValidator(new QIntValidator(0, 0xffff, this));

    bool contactsSelect;
    bool extCSSelect;
    bool ADIFSelect;
    QString contactsAddr;
    int contactsPort;
    QString extCSAddr;
    int extCSPort;
    QString ADIFAddr;
    int ADIFPort;

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpcontactsSelect, contactsSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpcontactsAddr, contactsAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpcontactsPort, contactsPort );
    ui->contactsSelect->setChecked(contactsSelect);
    ui->contactsAddr->setText(contactsAddr);
    ui->contactsPort->setText(QString::number(contactsPort));

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpextCSSelect, extCSSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpextCSAddr, extCSAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpextCSPort, extCSPort );
    ui->extCSSelect->setChecked(extCSSelect);
    ui->extCSAddr->setText(extCSAddr);
    ui->extCSPort->setText(QString::number(extCSPort));

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpADIFSelect, ADIFSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpADIFAddr, ADIFAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpADIFPort, ADIFPort );
    ui->ADIFSelect->setChecked(ADIFSelect);
    ui->ADIFAddr->setText(ADIFAddr);
    ui->ADIFPort->setText(QString::number(ADIFPort));
}

void N1MMBroadcastConfig::finalise()
{
    bool contactsSelect = ui->contactsSelect->isChecked();
    QString contactsAddr = ui->contactsAddr->text();
    int contactsPort = ui->contactsPort->text().toInt();
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpcontactsSelect, contactsSelect );
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpcontactsAddr, contactsAddr );
    TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpcontactsPort, contactsPort );

    bool extCSSelect = ui->extCSSelect->isChecked();
    QString extCSAddr = ui->extCSAddr->text();
    int extCSPort = ui->extCSPort->text().toInt();
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpextCSSelect, extCSSelect );
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpextCSAddr, extCSAddr );
    TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpextCSPort, extCSPort );

    bool ADIFSelect = ui->ADIFSelect->isChecked();
    QString ADIFAddr = ui->ADIFAddr->text();
    int ADIFPort = ui->ADIFPort->text().toInt();
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpADIFSelect, ADIFSelect );
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpADIFAddr, ADIFAddr );
    TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpADIFPort, ADIFPort );

    LogContainer->n1mmBroadcast.configure();
}
