#include "minos_pch.h"

#include "MinosLink.h"
#include "clientThread.h"
#include "serverThread.h"
#include "MServerZConf.h"
#include "MServer.h"
#include "MServerPubSub.h"

#include "servermain.h"

#include "ServerDetails.h"
#include "ui_ServerDetails.h"

// display class/form showing current connections and topics

ServerDetails::ServerDetails(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDetails)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QSettings settings;
    QByteArray geometry = settings.value("geometry/Details").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

}

ServerDetails::~ServerDetails()
{
    delete ui;
}

void ServerDetails::on_closeButton_clicked()
{
    hide();
}
void ServerDetails::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Details", saveGeometry());
    QWidget::resizeEvent(event);
}
void ServerDetails::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Details", saveGeometry());
    QWidget::moveEvent(event);
}
void ServerDetails::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
void ServerDetails::refresh()
{

    MinosRouterListener *msl = MinosRouterListener::getListener();
    ui->serversTable->clear();
    msl->buildTable(ui->serversTable);

    MinosClientListener *mcl = MinosClientListener::getListener();
    ui->clientsTable->clear();
    mcl->buildTable(ui->clientsTable);

    ui->publishedTable->clear();
    buildPublishedTree(ui->publishedTable);

    ui->subscribedTable->clear();
    buildSubscribedTree(ui->subscribedTable);
    // void TPubSubMain::revokeClient(const QString &pubId) give the way to iterate the pubs

    // Subscribed
    // PublishedCategory
    // SubscriberList subscribedLocal;
    // RemoteSubscriberList subscribedRemote;
    // ServerSubscriberList subscribedServer;
    // all in mserverpubsub.cpp
}
void ServerDetails::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    refresh();

}
void ServerDetails::on_refreshButton_clicked()
{
    refresh();
}
