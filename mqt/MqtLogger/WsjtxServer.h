#ifndef WSJTXSERVER_H
#define WSJTXSERVER_H

#include "base_pch.h"
#include "WsjtxMessageServer.hpp"
#include "WsjtxRadio.hpp"

using Frequency = MessageServer::Frequency;
using port_type = MessageServer::port_type;

class WsjtxServer : public QObject
{
    Q_OBJECT
public:
    WsjtxServer();
    static WsjtxServer *getWsjtxServer();

    void start ();

    void reply (QString const& id, QTime time, qint32 snr, float delta_time, quint32 delta_frequency
                       , QString const& mode, QString const& message, bool low_confidence, quint8 modifiers);

    void do_halt_tx (QString const& id, bool auto_only);
    void do_clear_decodes (QString const& id, quint8 window);

private:
    static WsjtxServer *wsjtxServer;
    MessageServer * server_1 = nullptr;
    MessageServer * server_2 = nullptr;
    MessageServer * server_3 = nullptr;
    MessageServer * server_4 = nullptr;
    bool fast_mode_ = false;
public slots:
    void add_client (QString const& id, QString const& version, QString const& revision);

    void remove_client (QString const& id);

//    void log_qso (QString const& /*id*/, QDateTime time_off, QString const& dx_call, QString const& dx_grid
//                         , Frequency dial_frequency, QString const& mode, QString const& report_sent
//                         , QString const& report_received, QString const& tx_power, QString const& comments
//                         , QString const& name, QDateTime time_on, QString const& operator_call
//                         , QString const& my_call, QString const& my_grid);

    void log_ADIF(QString const& id, QByteArray const& ADIF);

    bool fast_mode () const {return fast_mode_;}

    void update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                               , QString const& report, QString const& tx_mode, bool tx_enabled
                               , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                               , QString const& de_call, QString const& de_grid, QString const& dx_grid
                               , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, qint8 special_op_mode);

    void decode_added (bool is_new, QString const& client_id, QTime time, qint32 snr
                              , float delta_time, quint32 delta_frequency, QString const& mode
                              , QString const& message, bool low_confidence, bool off_air);
    void decodes_cleared (QString const& client_id);

signals:
    void do_add_client (QString const& id, QString const& version, QString const& revision);

    void do_remove_client (QString const& id);
    void do_free_text (QString const& id, QString const& text, bool);

    void do_decode_added (bool is_new, QString const& client_id, QTime, qint32 snr
                              , float delta_time, quint32 delta_frequency, QString const& mode
                              , QString const& message, bool low_confidence, bool off_air);
    void do_decodes_cleared (QString const& client_id);

//    void do_log_qso ( QString const& id, QDateTime time_off, QString const& dx_call, QString const& dx_grid
//                         , Frequency dial_frequency, QString const& mode, QString const& report_sent
//                         , QString const& report_received, QString const& tx_power, QString const& comments
//                         , QString const& name, QDateTime time_on, QString const& operator_call
//                         , QString const& my_call, QString const& my_grid);

    void do_log_ADIF(QString const& id, QByteArray const& ADIF);

    void do_update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                               , QString const& report, QString const& tx_mode, bool tx_enabled
                               , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                               , QString const& de_call, QString const& de_grid, QString const& dx_grid
                               , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, qint8 special_op_mode);
    void location (QString const& id, QString const& text);
    void highlight_callsign (QString const& id, QString const& call
                                      , QColor const& bg, QColor const& fg
                                      , bool last_only = false);

};

#endif // WSJTXSERVER_H
