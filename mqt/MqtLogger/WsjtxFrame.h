#ifndef WSJTXFRAME_H
#define WSJTXFRAME_H

#include "base_pch.h"

#include "WsjtxDecode.h"
#include "WsjtxDecodesModel.hpp"
#include "WsjtxRadio.hpp"
#include "WsjtxMessageServer.hpp"
#include "WsjtxServer.h"

using Frequency = MessageServer::Frequency;
using port_type = MessageServer::port_type;

namespace Ui {
class WsjtxFrame;
}

class WsjtxFrame : public QFrame
{
    Q_OBJECT

public:
    explicit WsjtxFrame(QWidget *parent = nullptr);
    ~WsjtxFrame();

    void setContest(BaseContestLog *c);
    SpecialOperatingActivity curOpMode = NONE;

    QVector<decodeMessage> messages;
    int decodeStartSize = 0;

private:
    Ui::WsjtxFrame *ui;
    BaseContestLog *ct = nullptr;
    DecodesModel * decodes_model_ = nullptr;
    bool fast_mode_ = false;
    QString id_;
    bool columns_resized_ = false;
    HtmlDelegate *delegate = nullptr;
    bool autoEnabled = false;
    bool showTest = false;
    bool inDecode = false;
    int lastcol = 0;
    QTime lastTime;

    WsjtxDecode decoder;

    void reply(decodeMessage &dc);
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
    void decode_added (bool is_new, const QString &id, QTime time, qint32 snr, float delta_time, quint32 delta_frequency,
                       const QString &mode, const QString &message, bool low_confidence, bool off_air);
    void decodes_cleared (QString const& client_id);

    void do_reply (QModelIndex source);

    void do_halt_tx (QString const& id, bool auto_only);

signals:
    void do_free_text (QString const& id, QString const& text, bool);
    void location (QString const& id, QString const& text);
    void highlight_callsign (QString const& id, QString const& call
                                      , QColor const& bg = QColor {}, QColor const& fg = QColor {}
                                      , bool last_only = false);
private slots:
    void on_autoSelectButton_toggled(bool);
    void on_testButton_clicked();
    void on_clearDecodesButton_clicked();
    void on_clearLocalDecodesButton_clicked();
};

#endif // WSJTXFRAME_H
