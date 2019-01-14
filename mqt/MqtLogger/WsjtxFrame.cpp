#include "ConfigFile.h"
#include "ContestApp.h"
#include "DecodesModel.hpp"

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

WsjtxFrame::IdFilterModel::IdFilterModel (QString const& client_id)
  : client_id_ {client_id}
  , rx_df_ (-1)
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
    , decodes_proxy_model_ {id_}
{
    ui->setupUi(this);
    decodes_proxy_model_.setSourceModel (decodes_model_);
    ui->decodes_table_view_->setModel (&decodes_proxy_model_);
    ui->decodes_table_view_->verticalHeader ()->hide ();
    ui->decodes_table_view_->hideColumn (0);
    ui->decodes_table_view_->horizontalHeader ()->setStretchLastSection (true);

connect (server_, &MessageServer::qso_logged, this, &WsjtxFrame::log_qso);
connect (server_, &MessageServer::client_opened, this, &WsjtxFrame::add_client);
connect (server_, &MessageServer::client_closed, this, &WsjtxFrame::remove_client);
connect (server_, &MessageServer::client_closed, decodes_model_, &DecodesModel::clear_decodes);
connect (server_, &MessageServer::decode, [this] (bool is_new, QString const& id, QTime time
                                                  , qint32 snr, float delta_time
                                                  , quint32 delta_frequency, QString const& mode
                                                  , QString const& message, bool low_confidence
                                                  , bool off_air) {
           decodes_model_->add_decode (is_new, id, time, snr, delta_time, delta_frequency, mode, message
                                       , low_confidence, off_air, fast_mode ());});
connect (server_, &MessageServer::clear_decodes, decodes_model_, &DecodesModel::clear_decodes);
connect (decodes_model_, &DecodesModel::reply, server_, &MessageServer::reply);

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
void WsjtxFrame::add_client (QString const& id, QString const& version, QString const& revision)
{
  connect (server_, &MessageServer::status_update, this, &WsjtxFrame::update_status);
  connect (server_, &MessageServer::decode, this, &WsjtxFrame::decode_added);
  connect (server_, &MessageServer::clear_decodes, this, &WsjtxFrame::clear_decodes);
  connect (this, &WsjtxFrame::do_reply, decodes_model_, &DecodesModel::do_reply);
  connect (this, &WsjtxFrame::do_halt_tx, server_, &MessageServer::halt_tx);
  connect (this, &WsjtxFrame::do_free_text, server_, &MessageServer::free_text);
  connect (this, &WsjtxFrame::location, server_, &MessageServer::location);
  connect (this, &WsjtxFrame::highlight_callsign, server_, &MessageServer::highlight_callsign);
  server_->replay (id);         // request decodes and status
}

void WsjtxFrame::remove_client (QString const& id)
{

}
void WsjtxFrame::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                  , QString const& report, QString const& tx_mode, bool tx_enabled
                                  , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                  , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                  , bool watchdog_timeout, QString const& sub_mode, bool fast_mode)
{
//  if (id == id_)
//    {
//      fast_mode_ = fast_mode;
//      decodes_proxy_model_.de_call (de_call);
//      decodes_proxy_model_.rx_df (rx_df);
//      de_label_->setText (de_call.size () >= 0 ? QString {"DE: %1%2"}.arg (de_call)
//                          .arg (de_grid.size () ? '(' + de_grid + ')' : QString {}) : QString {});
//      mode_label_->setText (QString {"Mode: %1%2%3%4"}
//           .arg (mode)
//           .arg (sub_mode)
//           .arg (fast_mode && !mode.contains (QRegularExpression {R"(ISCAT|MSK144)"}) ? "fast" : "")
//           .arg (tx_mode.isEmpty () || tx_mode == mode ? "" : '(' + tx_mode + ')'));
//      frequency_label_->setText ("QRG: " + Radio::pretty_frequency_MHz_string (f));
//      dx_label_->setText (dx_call.size () >= 0 ? QString {"DX: %1%2"}.arg (dx_call)
//                          .arg (dx_grid.size () ? '(' + dx_grid + ')' : QString {}) : QString {});
//      rx_df_label_->setText (rx_df >= 0 ? QString {"Rx: %1"}.arg (rx_df) : "");
//      tx_df_label_->setText (tx_df >= 0 ? QString {"Tx: %1"}.arg (tx_df) : "");
//      report_label_->setText ("SNR: " + report);
//      update_dynamic_property (frequency_label_, "transmitting", transmitting);
//      auto_off_button_->setEnabled (tx_enabled);
//      halt_tx_button_->setEnabled (transmitting);
//      update_dynamic_property (mode_label_, "decoding", decoding);
//      update_dynamic_property (tx_df_label_, "watchdog_timeout", watchdog_timeout);
//    }
}

void WsjtxFrame::decode_added (bool /*is_new*/, QString const& client_id, QTime /*time*/, qint32 /*snr*/
                                 , float /*delta_time*/, quint32 /*delta_frequency*/, QString const& /*mode*/
                                 , QString const& /*message*/, bool /*low_confidence*/, bool /*off_air*/)
{
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
  if (client_id == id_)
    {
      //columns_resized_ = false;
    }
}
