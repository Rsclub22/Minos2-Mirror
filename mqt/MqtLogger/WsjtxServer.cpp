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
  server_1 {new MessageServer {this}}
, server_2 {new MessageServer {this}}
, server_3 {new MessageServer {this}}
, server_4 {new MessageServer {this}}
{
    connect (server_1, &MessageServer::status_update, this, &WsjtxServer::update_status);
    //connect (server_, &MessageServer::qso_logged, this, &WsjtxServer::log_qso);
    connect (server_1, &MessageServer::logged_ADIF, this, &WsjtxServer::log_ADIF);
    connect (server_1, &MessageServer::client_opened, this, &WsjtxServer::add_client);
    connect (server_1, &MessageServer::client_closed, this, &WsjtxServer::remove_client);
    connect (server_1, &MessageServer::decode, this, &WsjtxServer::decode_added);
    connect (server_1, &MessageServer::decodes_cleared,  this, &WsjtxServer::decodes_cleared);

    connect (server_2, &MessageServer::logged_ADIF, this, &WsjtxServer::log_ADIF);
    connect (server_3, &MessageServer::logged_ADIF, this, &WsjtxServer::log_ADIF);
    connect (server_4, &MessageServer::logged_ADIF, this, &WsjtxServer::log_ADIF);
}
void WsjtxServer::start ( )
{
    {
        bool enabled;
        int port = 0;
        QString multicast_group_address;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX1Enabled, enabled );
        if (enabled)
        {
            TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTX1Port, port );
            TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX1GroupAddress, multicast_group_address );

            trace(QString("WsjtxServer::start  1 port %1 address %2").arg(port).arg(multicast_group_address));

            server_1->start(static_cast<port_type>( port), QHostAddress {multicast_group_address});
        }
        else
        {
            // start port 0 should stop it
            server_1->start(static_cast<port_type>( 0), QHostAddress {multicast_group_address});
        }
    }
    {
        bool enabled;
        int port = 0;
        QString multicast_group_address;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX2Enabled, enabled );
        if (enabled)
        {
            TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTX2Port, port );
            TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX2GroupAddress, multicast_group_address );

            trace(QString("WsjtxServer::start 2 port %1 address %2").arg(port).arg(multicast_group_address));

            server_2->start(static_cast<port_type>( port), QHostAddress {multicast_group_address});
        }
        else
        {
            // start port 0 should stop it
            server_2->start(static_cast<port_type>( 0), QHostAddress {multicast_group_address});
        }
    }
    {
        bool enabled;
        int port = 0;
        QString multicast_group_address;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX3Enabled, enabled );
        if (enabled)
        {
            TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTX3Port, port );
            TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX3GroupAddress, multicast_group_address );

            trace(QString("WsjtxServer::start 3 port %1 address %2").arg(port).arg(multicast_group_address));

            server_3->start(static_cast<port_type>( port), QHostAddress {multicast_group_address});
        }
        else
        {
            // start port 0 should stop it
            server_3->start(static_cast<port_type>( 0), QHostAddress {multicast_group_address});
        }
    }
    {
        bool enabled;
        int port = 0;
        QString multicast_group_address;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX4Enabled, enabled );
        if (enabled)
        {
            TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpWSJTX4Port, port );
            TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX4GroupAddress, multicast_group_address );

            trace(QString("WsjtxServer::start 4 port %1 address %2").arg(port).arg(multicast_group_address));

            server_4->start(static_cast<port_type>( port), QHostAddress {multicast_group_address});
        }
        else
        {
            // start port 0 should stop it
            server_4->start(static_cast<port_type>( 0), QHostAddress {multicast_group_address});
        }
    }
}
//void WsjtxServer::log_qso (QString const& id, QDateTime time_off, QString const& dx_call
//                                           , QString const& dx_grid, Frequency dial_frequency, QString const& mode
//                                           , QString const& report_sent, QString const& report_received
//                                           , QString const& tx_power, QString const& comments
//                                           , QString const& name, QDateTime time_on, QString const& operator_call
//                                           , QString const& my_call, QString const& my_grid)
//{
//    emit do_log_qso(id, time_off, dx_call, dx_grid, dial_frequency, mode, report_sent, report_received, tx_power, comments, name, time_on, operator_call
//                 , my_call, my_grid);
//}
void WsjtxServer::log_ADIF(QString const& id, QByteArray const& ADIF)
{
    //trace(QString("WsjtxServer::log_ADIF %1").arg(QString(ADIF)));
    emit do_log_ADIF(id, ADIF);
}
void WsjtxServer::add_client (QString const& id, QString const& version, QString const& revision)
{
    trace(QString("WsjtxServer::add_client"));
    emit do_add_client(id, version, revision);
    server_1->replay (id);         // request decodes and status
}

void WsjtxServer::remove_client (QString const& id)
{
    trace(QString("WsjtxServer::remove_client"));
    emit do_remove_client(id);
}
void WsjtxServer::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                  , QString const& report, QString const& tx_mode, bool tx_enabled
                                  , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                  , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                  , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, qint8 special_op_mode)
{
    emit do_update_status(id, f, mode, dx_call, report, tx_mode, tx_enabled, transmitting, decoding, rx_df, tx_df,
                          de_call, de_grid, dx_grid, watchdog_timeout, sub_mode, fast_mode, special_op_mode);

    //trace(QString("WsjtxServer::update_status transmitting %1 decoding %2").arg(transmitting).arg(decoding));

}

void WsjtxServer::decode_added (bool is_new, QString const& client_id, QTime time, qint32 snr
                                 , float delta_time, quint32 delta_frequency, QString const& mode
                                 , QString const& message, bool low_confidence, bool off_air)
{
    //trace(QString("WsjtxServer::decode_added %1").arg(message));
    emit do_decode_added(is_new, client_id, time, snr, delta_time, delta_frequency, mode, message, low_confidence, off_air);
}
void WsjtxServer::decodes_cleared (QString const& client_id)
{
    trace(QString("WsjtxServer::decodes_cleared"));
    emit do_decodes_cleared(client_id);
}
void WsjtxServer::reply (QString const& id, QTime time, qint32 snr, float delta_time, quint32 delta_frequency
                   , QString const& mode, QString const& message, bool low_confidence, quint8 modifiers)
{
    trace(QString("WsjtxServer::reply to " + message));
    server_1->reply(id, time, snr, delta_time, delta_frequency, mode, message, low_confidence, modifiers);
}

void WsjtxServer::do_halt_tx (QString const& id, bool auto_only)
{
    if (auto_only)
        trace("WsjtxServer Disable TX");
    else
        trace("WsjtxServer Halt TX");

    server_1->halt_tx(id, auto_only);
}
void WsjtxServer::do_clear_decodes (QString const& id, quint8 window)
{
    server_1->clear_decodes(id, window);
}
