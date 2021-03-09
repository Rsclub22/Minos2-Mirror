#include "wsjtxlink.h"

WsjtxLink::WsjtxLink(QObject *parent):
    QObject(parent),
    msgServer {new MessageServer {this}}

{
    connect (msgServer, &MessageServer::status_update, this, &WsjtxLink::update_status);
    //connect (server_, &MessageServer::qso_logged, this, &WsjtxServer::log_qso);
    connect (msgServer, &MessageServer::logged_ADIF, this, &WsjtxLink::log_ADIF);
    connect (msgServer, &MessageServer::client_opened, this, &WsjtxLink::add_client);
    connect (msgServer, &MessageServer::client_closed, this, &WsjtxLink::remove_client);
    connect (msgServer, &MessageServer::decode, this, &WsjtxLink::decode_added);
    connect (msgServer, &MessageServer::decodes_cleared,  this, &WsjtxLink::decodes_cleared);


}
WsjtxLink::~WsjtxLink()
{
    msgServer->stop();
    msgServer->deleteLater();
}
void WsjtxLink::initialise()
{
    int port = 0;
    QString multicast_group_address;
    QSettings settings;

    port = settings.value("WSJT-X port", 2237).toInt();
    multicast_group_address = settings.value( "WSJT-X address", "" ).toString();

    trace(QString("Wsjtxlink::start  1 port %1 address %2").arg(port).arg(multicast_group_address));

    msgServer->start(static_cast<MessageServer::port_type>( port), QHostAddress {multicast_group_address});
}

void WsjtxLink::disconnect()
{
    msgServer->stop();
}
void WsjtxLink::log_ADIF(QString const& id, QByteArray const& ADIF)
{
    //trace(QString("WsjtxServer::log_ADIF %1").arg(QString(ADIF)));
    emit do_log_ADIF(id, ADIF);
}
void WsjtxLink::add_client (QString const& id, QString const& version, QString const& revision)
{
    trace(QString("WsjtxServer::add_client"));
    emit do_add_client(id, version, revision);
}

void WsjtxLink::remove_client (QString const& id)
{
    trace(QString("WsjtxServer::remove_client"));
    emit do_remove_client(id);
}
void WsjtxLink::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                  , QString const& report, QString const& tx_mode, bool tx_enabled
                                  , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                  , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                  , bool watchdog_timeout, QString const& sub_mode, bool fast_mode
                                  , quint8 special_op_mode, quint32 frequency_tolerance, quint32 tr_period
                                  , QString const& configuration_name, QString const& tx_message)
{
    emit do_update_status(id, f, mode, dx_call, report, tx_mode, tx_enabled, transmitting, decoding, rx_df, tx_df,
                          de_call, de_grid, dx_grid, watchdog_timeout, sub_mode, fast_mode, special_op_mode,
                          frequency_tolerance, tr_period, configuration_name, tx_message);

    //trace(QString("WsjtxServer::update_status transmitting %1 decoding %2").arg(transmitting).arg(decoding));

}

void WsjtxLink::decode_added (bool is_new, QString const& client_id, QTime time, qint32 snr
                                 , float delta_time, quint32 delta_frequency, QString const& mode
                                 , QString const& message, bool low_confidence, bool off_air)
{
    //trace(QString("WsjtxServer::decode_added %1").arg(message));
    emit do_decode_added(is_new, client_id, time, snr, delta_time, delta_frequency, mode, message, low_confidence, off_air);
}
void WsjtxLink::decodes_cleared (QString const& client_id)
{
    trace(QString("WsjtxServer::decodes_cleared"));
    emit do_decodes_cleared(client_id);
}
