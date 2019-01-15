#include "ConfigFile.h"
#include "ContestApp.h"

#include "WsjtxServer.h"

WsjtxServer *WsjtxServer::wsjtxServer = nullptr;


WsjtxServer *WsjtxServer::getWsjtxServer()
{
    if (!wsjtxServer)
    {
        wsjtxServer = new WsjtxServer();
    }
    return wsjtxServer;
}
WsjtxServer::WsjtxServer():
 server_ {new MessageServer {this}}
{
    connect (server_, &MessageServer::status_update, this, &WsjtxServer::update_status);
    connect (server_, &MessageServer::qso_logged, this, &WsjtxServer::log_qso);
    connect (server_, &MessageServer::logged_ADIF, this, &WsjtxServer::log_ADIF);
    connect (server_, &MessageServer::client_opened, this, &WsjtxServer::add_client);
    connect (server_, &MessageServer::client_closed, this, &WsjtxServer::remove_client);
    connect (server_, &MessageServer::decode, this, &WsjtxServer::decode_added);
    connect (server_, &MessageServer::clear_decodes,  this, &WsjtxServer::clear_decodes);

}
void WsjtxServer::start (port_type port,
                     QHostAddress const& multicast_group_address )
{
    server_->start(port, multicast_group_address);
}
void WsjtxServer::log_qso (QString const& id, QDateTime time_off, QString const& dx_call
                                           , QString const& dx_grid, Frequency dial_frequency, QString const& mode
                                           , QString const& report_sent, QString const& report_received
                                           , QString const& tx_power, QString const& comments
                                           , QString const& name, QDateTime time_on, QString const& operator_call
                                           , QString const& my_call, QString const& my_grid)
{
    emit do_log_qso(id, time_off, dx_call, dx_grid, dial_frequency, mode, report_sent, report_received, tx_power, comments, name, time_on, operator_call
                 , my_call, my_grid);
}
void WsjtxServer::log_ADIF(QString const& id, QByteArray const& ADIF)
{
    emit do_log_ADIF(id, ADIF);
}
void WsjtxServer::add_client (QString const& id, QString const& version, QString const& revision)
{
    emit do_add_client(id, version, revision);
    server_->replay (id);         // request decodes and status
}

void WsjtxServer::remove_client (QString const& id)
{
    emit do_remove_client(id);
}
void WsjtxServer::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                  , QString const& report, QString const& tx_mode, bool tx_enabled
                                  , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                  , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                  , bool watchdog_timeout, QString const& sub_mode, bool fast_mode)
{
    emit do_update_status(id, f, mode, dx_call, report, tx_mode, tx_enabled, transmitting, decoding, rx_df, tx_df,
                          de_call, de_grid, dx_grid, watchdog_timeout, sub_mode, fast_mode);
}

void WsjtxServer::decode_added (bool is_new, QString const& client_id, QTime time, qint32 snr
                                 , float delta_time, quint32 delta_frequency, QString const& mode
                                 , QString const& message, bool low_confidence, bool off_air)
{
    emit do_decode_added(is_new, client_id, time, snr, delta_time, delta_frequency, mode, message, low_confidence, off_air);
}
void WsjtxServer::clear_decodes (QString const& client_id)
{
    emit do_clear_decodes(client_id);
}
void WsjtxServer::reply (QString const& id, QTime time, qint32 snr, float delta_time, quint32 delta_frequency
                   , QString const& mode, QString const& message, bool low_confidence, quint8 modifiers)
{
    server_->reply(id, time, snr, delta_time, delta_frequency, mode, message, low_confidence, modifiers);
}
