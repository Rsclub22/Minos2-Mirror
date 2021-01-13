#ifndef WSJTXLINK_H
#define WSJTXLINK_H

#include "base_pch.h"
#include "WsjtxMessageServer.hpp"

class WsjtxLink:public QObject
{
    Q_OBJECT
public:
    WsjtxLink(QObject *parent);
    virtual ~WsjtxLink() override;

    void initialise();

    void disconnect();

private:
    MessageServer * server = nullptr;

private slots:
    void add_client (QString const& id, QString const& version, QString const& revision);

    void remove_client (QString const& id);

//    void log_qso (QString const& /*id*/, QDateTime time_off, QString const& dx_call, QString const& dx_grid
//                         , Frequency dial_frequency, QString const& mode, QString const& report_sent
//                         , QString const& report_received, QString const& tx_power, QString const& comments
//                         , QString const& name, QDateTime time_on, QString const& operator_call
//                         , QString const& my_call, QString const& my_grid);

    void log_ADIF(QString const& id, QByteArray const& ADIF);

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

    void do_decode_added (bool is_new, QString const& client_id, QTime, qint32 snr
                              , float delta_time, quint32 delta_frequency, QString const& mode
                              , QString const& message, bool low_confidence, bool off_air);
    void do_decodes_cleared (QString const& client_id);

    void do_log_ADIF(QString const& id, QByteArray const& ADIF);

    void do_update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                               , QString const& report, QString const& tx_mode, bool tx_enabled
                               , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                               , QString const& de_call, QString const& de_grid, QString const& dx_grid
                               , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, qint8 special_op_mode);
};

#endif // WSJTXLINK_H
