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

    QString contactsAddr;
    int contactsPort;
    QString extCSAddr;
    int extCSPort;
    QString ADIFAddr;
    int ADIFPort;

    contactsSelect.initialise(&TContestApp::getContestApp() ->loggerBundle, elpcontactsSelect,  ui->contactsSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpcontactsAddr, contactsAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpcontactsPort, contactsPort );
    ui->contactsAddr->setText(contactsAddr);
    ui->contactsPort->setText(QString::number(contactsPort));

    extCSSelect.initialise(&TContestApp::getContestApp() ->loggerBundle, elpextCSSelect,  ui->extCSSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpextCSAddr, extCSAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpextCSPort, extCSPort );
    ui->extCSAddr->setText(extCSAddr);
    ui->extCSPort->setText(QString::number(extCSPort));

    ADIFSelect.initialise(&TContestApp::getContestApp() ->loggerBundle, elpADIFSelect,  ui->ADIFSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpADIFAddr, ADIFAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpADIFPort, ADIFPort );
    ui->ADIFAddr->setText(ADIFAddr);
    ui->ADIFPort->setText(QString::number(ADIFPort));
}
bool N1MMBroadcastConfig::check()
{
    return true;
}
void N1MMBroadcastConfig::cancel()
{

}

void N1MMBroadcastConfig::finalise()
{
    contactsSelect.finalise();
    QString contactsAddr = ui->contactsAddr->text();
    int contactsPort = ui->contactsPort->text().toInt();
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpcontactsAddr, contactsAddr );
    TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpcontactsPort, contactsPort );

    extCSSelect.finalise();
    QString extCSAddr = ui->extCSAddr->text();
    int extCSPort = ui->extCSPort->text().toInt();
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpextCSAddr, extCSAddr );
    TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpextCSPort, extCSPort );

    ADIFSelect.finalise();
    QString ADIFAddr = ui->ADIFAddr->text();
    int ADIFPort = ui->ADIFPort->text().toInt();
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpADIFAddr, ADIFAddr );
    TContestApp::getContestApp() ->loggerBundle.setIntProfile( elpADIFPort, ADIFPort );

    LogContainer->n1mmBroadcast.configure();
}
