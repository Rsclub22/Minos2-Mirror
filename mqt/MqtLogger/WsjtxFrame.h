#ifndef WSJTXFRAME_H
#define WSJTXFRAME_H

#include "base_pch.h"

#include "DecodesModel.hpp"
#include "Radio.hpp"
#include "MessageServer.hpp"
#include "WsjtxServer.h"

using Frequency = MessageServer::Frequency;
using port_type = MessageServer::port_type;

namespace Ui {
class WsjtxFrame;
}
/*
  // Normal
CQ K1ABC FN42                          #K1ABC calls CQ
                  K1ABC G0XYZ IO91     #G0XYZ answers
G0XYZ K1ABC –19                        #K1ABC sends report
                  K1ABC G0XYZ R-22     #G0XYZ sends R+report
G0XYZ K1ABC RR73                       #K1ABC sends RRR - or RR73
                  K1ABC G0XYZ 73       #G0XYZ sends 73
  */

/*
  // EU VHF Contest
CQ TEST G4ABC/P IO91                                         "/P" is optional
                                   G4ABC/P PA9XYZ JO22       on either callsign
PA9XYZ 590003 IO91NP
                                   G4ABC/P R 570007 JO22DB
PA9XYZ G4ABC/P RR73
                                   G4ABC/P PA9XYZ 73
  */
enum MessageStage {emsNone, emsCQ, emsGrid, emsDb, emsRplusDb, emsRRR, ems73};
class WsjtxFrame;
class decodeMessage
{
public:
    // Can I populate this accurately?
    // Do I need the whole decode set to allow for reply?
    // Doesn't contain MY sent messages... In fact, once working someone
    // it may all go to pot

    // I CAN see transmit "sessions" - does the status give me enough?

    MessageStage mstage{emsNone};
    SpecialOperatingActivity opMode = NONE;

    QTime decodeTime;
    QString message;
    QString toCall;
    QString toGrid;
    QString fromCall;
    QString fromGrid;
    int strength = -100;
    int bearing = 0;
    int distance = 0;
    int points = 0;

    decodeMessage(WsjtxFrame *frame, bool is_new, QString const& id, QTime time
                  , qint32 snr, float delta_time
                  , quint32 delta_frequency, QString const& mode
                  , QString const& message, bool low_confidence
                  , bool off_air);
    decodeMessage();
    ~decodeMessage();
};

class WsjtxFrame : public QFrame
{
    Q_OBJECT

public:
    explicit WsjtxFrame(QWidget *parent = nullptr);
    ~WsjtxFrame();

    void setContest(BaseContestLog *c);
    SpecialOperatingActivity curOpMode = NONE;
    QString myCall;
    QString myLoc;

    QVector<decodeMessage> messages;

private:
    Ui::WsjtxFrame *ui;
    BaseContestLog *ct = nullptr;
    DecodesModel * decodes_model_ = nullptr;
    bool fast_mode_ = false;
    QString id_;
    bool columns_resized_ = false;
    HtmlDelegate *delegate = nullptr;


    class IdFilterModel final
      : public QSortFilterProxyModel
    {
    public:
      IdFilterModel ();

      void de_call (QString const&);
      void rx_df (int);
      void setId(QString clientId)
      {
          client_id_ = clientId;
      }

      QVariant data (QModelIndex const& proxy_index, int role = Qt::DisplayRole) const override;

    protected:
      bool filterAcceptsRow (int source_row, QModelIndex const& source_parent) const override;

    private:
      QString client_id_;
      QString call_;
      QRegularExpression base_call_re_;
      int rx_df_;
    } decodes_proxy_model_;



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
    void clear_decodes (QString const& client_id);

    void reply (QString const& id, QTime time, qint32 snr, float delta_time
                               , quint32 delta_frequency, QString const& mode
                               , QString const& message_text, bool low_confidence, quint8 modifiers);

signals:
    void do_halt_tx (QString const& id, bool auto_only);
    void do_free_text (QString const& id, QString const& text, bool);
    void location (QString const& id, QString const& text);
    void highlight_callsign (QString const& id, QString const& call
                                      , QColor const& bg = QColor {}, QColor const& fg = QColor {}
                                      , bool last_only = false);
private slots:
    void on_autoSelectButton_clicked();
};

#endif // WSJTXFRAME_H
