#include "ConfigFile.h"
#include "ContestApp.h"
#include "DecodesModel.hpp"
#include "WsjtxServer.h"

#include "WsjtxFrame.h"
#include "ui_WsjtxFrame.h"
static QString wsjtStateIndicator[] =
{
    "Available",
    "NotAvailable",
    "NoContact"
};
static QString wsjtStateList[] =
{
   "Available",
   "Not Available",
   "No Contact"
};
namespace
{
  //QRegExp message_alphabet {"[- A-Za-z0-9+./?]*"};
  QRegExp message_alphabet {"[- @A-Za-z0-9+./?#<>]*"};
  QRegularExpression cq_re {"(CQ|CQDX|QRZ)[^A-Z0-9/]+"};

  void update_dynamic_property (QWidget * widget, char const * property, QVariant const& value)
  {
    widget->setProperty (property, value);
    widget->style ()->unpolish (widget);
    widget->style ()->polish (widget);
    widget->update ();
  }
}

WsjtxFrame::IdFilterModel::IdFilterModel ()
  : rx_df_ (-1)
{
}

QVariant WsjtxFrame::IdFilterModel::data (QModelIndex const& proxy_index, int role) const
{
  if (role == Qt::BackgroundRole)
    {
      switch (proxy_index.column ())
        {
        case 8:                 // message
          {
            auto message = QSortFilterProxyModel::data (proxy_index).toString ();
            if (base_call_re_.pattern ().size ()
                && message.contains (base_call_re_))
              {
                return QColor {255,200,200};
              }
            if (message.contains (cq_re))
              {
                return QColor {200, 255, 200};
              }
          }
          break;

        case 4:                 // DF
          if (qAbs (QSortFilterProxyModel::data (proxy_index).toInt () - rx_df_) <= 10)
            {
              return QColor {255, 200, 200};
            }
          break;

        default:
          break;
        }
    }
  return QSortFilterProxyModel::data (proxy_index, role);
}

bool WsjtxFrame::IdFilterModel::filterAcceptsRow (int source_row
                                                    , QModelIndex const& source_parent) const
{
  auto source_index_col0 = sourceModel ()->index (source_row, 0, source_parent);
  return sourceModel ()->data (source_index_col0).toString () == client_id_;
}

void WsjtxFrame::IdFilterModel::de_call (QString const& call)
{
  if (call != call_)
    {
      beginResetModel ();
      if (call.size ())
        {
          base_call_re_.setPattern ("[^A-Z0-9]*" + Radio::base_callsign (call) + "[^A-Z0-9]*");
        }
      else
        {
          base_call_re_.setPattern (QString {});
        }
      call_ = call;
      endResetModel ();
    }
}

void WsjtxFrame::IdFilterModel::rx_df (int df)
{
  if (df != rx_df_)
    {
      beginResetModel ();
      rx_df_ = df;
      endResetModel ();
    }
}

namespace
{
  QString make_title (QString const& id, QString const& version, QString const& revision)
  {
    QString title {id};
    if (version.size ())
      {
        title += QString {" v%1"}.arg (version);
      }
    if (revision.size ())
      {
        title += QString {" (%1)"}.arg (revision);
      }
    return "Client: " + title;
  }
}

WsjtxFrame::WsjtxFrame(QWidget *parent) :
    QFrame(parent)
  , ui(new Ui::WsjtxFrame)
  , decodes_model_ {new DecodesModel {this}}
  , decodes_proxy_model_ {}

{
    ui->setupUi(this);

    ui->port_spin_box->setMinimum (1);
    ui->port_spin_box->setMaximum (std::numeric_limits<port_type>::max ());

    decodes_proxy_model_.setSourceModel (decodes_model_);
    ui->decodes_table_view_->setModel (&decodes_proxy_model_);
    ui->decodes_table_view_->verticalHeader ()->hide ();
    ui->decodes_table_view_->hideColumn (0);
    ui->decodes_table_view_->horizontalHeader ()->setStretchLastSection (true);

    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_log_qso, this, &WsjtxFrame::log_qso);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_add_client, this, &WsjtxFrame::add_client);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_remove_client, this, &WsjtxFrame::remove_client);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_remove_client, decodes_model_, &DecodesModel::clear_decodes);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_decode_added, [this] (bool is_new, QString const& id, QTime time
             , qint32 snr, float delta_time
             , quint32 delta_frequency, QString const& mode
             , QString const& message, bool low_confidence
             , bool off_air) {
        decodes_model_->add_decode (is_new, id, time, snr, delta_time, delta_frequency, mode, message
                                    , low_confidence, off_air, fast_mode ());});
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_clear_decodes, decodes_model_, &DecodesModel::clear_decodes);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_update_status, this, &WsjtxFrame::update_status);
    connect (decodes_model_, &DecodesModel::reply, this, &WsjtxFrame::reply);

    // UI behaviour
    connect (ui->port_spin_box, static_cast<void (QSpinBox::*)(int)> (&QSpinBox::valueChanged)
             , [] (port_type port) {WsjtxServer::getWsjtxServer()->start (port);});

    connect (ui->multicast_group_line_edit_, &QLineEdit::editingFinished, [this] () {
        WsjtxServer::getWsjtxServer()->start (ui->port_spin_box->value (), QHostAddress {ui->multicast_group_line_edit_->text ()});
      });

    ui->port_spin_box->setValue (2237); // start up in unicast mode

    connect (ui->auto_off_button_, &QAbstractButton::clicked, [this] (bool /* checked */) {
        emit do_halt_tx (id_, true);
      });
    connect (ui->halt_tx_button_, &QAbstractButton::clicked, [this] (bool /* checked */) {
        emit do_halt_tx (id_, false);
      });


}
WsjtxFrame::~WsjtxFrame()
{
    delete ui;
}

void WsjtxFrame::setContest(BaseContestLog *c)
{
    ct = c;
}

void WsjtxFrame::log_qso (QString const& /*id*/, QDateTime time_off, QString const& dx_call
                                           , QString const& dx_grid, Frequency dial_frequency, QString const& mode
                                           , QString const& report_sent, QString const& report_received
                                           , QString const& tx_power, QString const& comments
                                           , QString const& name, QDateTime time_on, QString const& operator_call
                                           , QString const& my_call, QString const& my_grid)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;
//  QList<QStandardItem *> row;
//  row << new QStandardItem {time_on.toString ("dd-MMM-yyyy hh:mm:ss")}
//  << new QStandardItem {time_off.toString ("dd-MMM-yyyy hh:mm:ss")}
//  << new QStandardItem {dx_call}
//  << new QStandardItem {dx_grid}
//  << new QStandardItem {name}
//  << new QStandardItem {Radio::frequency_MHz_string (dial_frequency)}
//  << new QStandardItem {mode}
//  << new QStandardItem {report_sent}
//  << new QStandardItem {report_received}
//  << new QStandardItem {tx_power}
//  << new QStandardItem {operator_call}
//  << new QStandardItem {my_call}
//  << new QStandardItem {my_grid}
//  << new QStandardItem {comments};
//  log_->appendRow (row);
//  log_table_view_->resizeColumnsToContents ();
//  log_table_view_->horizontalHeader ()->setStretchLastSection (true);
//  log_table_view_->scrollToBottom ();
}
void WsjtxFrame::log_ADIF(QString const& id, QByteArray const& ADIF)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;
}
void WsjtxFrame::add_client (QString const& id, QString const& /*version*/, QString const& /*revision*/)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

    id_ = id;
}

void WsjtxFrame::remove_client (QString const& /*id*/)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;    id_.clear();
}
void WsjtxFrame::update_status (QString const& /*id*/, Frequency f, QString const& mode, QString const& dx_call
                                  , QString const& report, QString const& tx_mode, bool tx_enabled
                                  , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                  , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                  , bool watchdog_timeout, QString const& sub_mode, bool fast_mode)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

    // pass to decodes model
//  if (id == id_)
//    {
      fast_mode_ = fast_mode;
      decodes_proxy_model_.de_call (de_call);
      decodes_proxy_model_.rx_df (rx_df);
      ui->de_label_->setText (de_call.size () >= 0 ? QString {"DE: %1%2"}.arg (de_call)
                          .arg (de_grid.size () ? '(' + de_grid + ')' : QString {}) : QString {});
      ui->mode_label_->setText (QString {"Mode: %1%2%3%4"}
           .arg (mode)
           .arg (sub_mode)
           .arg (fast_mode && !mode.contains (QRegularExpression {R"(ISCAT|MSK144)"}) ? "fast" : "")
           .arg (tx_mode.isEmpty () || tx_mode == mode ? "" : '(' + tx_mode + ')'));
      ui->frequency_label_->setText ("QRG: " + Radio::pretty_frequency_MHz_string (f));
      ui->dx_label_->setText (dx_call.size () >= 0 ? QString {"DX: %1%2"}.arg (dx_call)
                          .arg (dx_grid.size () ? '(' + dx_grid + ')' : QString {}) : QString {});
      ui->rx_df_label_->setText (rx_df >= 0 ? QString {"Rx: %1"}.arg (rx_df) : "");
      ui->tx_df_label_->setText (tx_df >= 0 ? QString {"Tx: %1"}.arg (tx_df) : "");
      ui->report_label_->setText ("SNR: " + report);
      update_dynamic_property (ui->frequency_label_, "transmitting", transmitting);
      ui->auto_off_button_->setEnabled (tx_enabled);
      ui->halt_tx_button_->setEnabled (transmitting);
      update_dynamic_property (ui->mode_label_, "decoding", decoding);
      update_dynamic_property (ui->tx_df_label_, "watchdog_timeout", watchdog_timeout);
//    }
}

void WsjtxFrame::decode_added (bool /*is_new*/, QString const& client_id, QTime /*time*/, qint32 /*snr*/
                                 , float /*delta_time*/, quint32 /*delta_frequency*/, QString const& /*mode*/
                                 , QString const& /*message*/, bool /*low_confidence*/, bool /*off_air*/)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

//  if (client_id == id_ && !columns_resized_)
//    {
//      decodes_stack_->setCurrentIndex (0);
//      decodes_table_view_->resizeColumnsToContents ();
//      columns_resized_ = true;
//    }
//  decodes_table_view_->scrollToBottom ();
}
void WsjtxFrame::clear_decodes (QString const& client_id)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

//    if (client_id == id_)
//    {
//      //columns_resized_ = false;
//    }
}
void WsjtxFrame::reply (QString const& id, QTime time, qint32 snr, float delta_time
                           , quint32 delta_frequency, QString const& mode
                           , QString const& message_text, bool low_confidence, quint8 modifiers)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

    WsjtxServer::getWsjtxServer()->reply(id, time, snr, delta_time, delta_frequency, mode, message_text, low_confidence, modifiers);
}
