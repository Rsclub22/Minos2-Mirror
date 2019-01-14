#ifndef WSJTXFRAME_H
#define WSJTXFRAME_H

#include "base_pch.h"

#include "DecodesModel.hpp"
#include "Radio.hpp"
#include "MessageServer.hpp"

using Frequency = MessageServer::Frequency;

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

private:
    Ui::WsjtxFrame *ui;
    BaseContestLog *ct = nullptr;
    DecodesModel * decodes_model_;
    MessageServer * server_;
    QString id_;
    bool fast_mode_;

    class IdFilterModel final
      : public QSortFilterProxyModel
    {
    public:
      IdFilterModel (QString const& client_id);

      void de_call (QString const&);
      void rx_df (int);

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

    void log_qso (QString const& /*id*/, QDateTime time_off, QString const& dx_call, QString const& dx_grid
                         , Frequency dial_frequency, QString const& mode, QString const& report_sent
                         , QString const& report_received, QString const& tx_power, QString const& comments
                         , QString const& name, QDateTime time_on, QString const& operator_call
                         , QString const& my_call, QString const& my_grid);

    bool fast_mode () const {return fast_mode_;}

    void update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                               , QString const& report, QString const& tx_mode, bool tx_enabled
                               , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                               , QString const& de_call, QString const& de_grid, QString const& dx_grid
                               , bool watchdog_timeout, QString const& sub_mode, bool fast_mode);
    void decode_added (bool is_new, QString const& client_id, QTime, qint32 snr
                              , float delta_time, quint32 delta_frequency, QString const& mode
                              , QString const& message, bool low_confidence, bool off_air);
    void clear_decodes (QString const& client_id);

signals:
    void do_reply (QModelIndex const&, quint8 modifier);
    void do_halt_tx (QString const& id, bool auto_only);
    void do_free_text (QString const& id, QString const& text, bool);
    void location (QString const& id, QString const& text);
    void highlight_callsign (QString const& id, QString const& call
                                      , QColor const& bg = QColor {}, QColor const& fg = QColor {}
                                      , bool last_only = false);


};

#endif // WSJTXFRAME_H
