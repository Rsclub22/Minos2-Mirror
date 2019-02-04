#include "ConfigFile.h"
#include "ContestApp.h"
#include "LoggerContest.h"
#include "AdifImport.h"
#include "MinosLoggerEvents.h"
#include "contacts.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "htmldelegate.h"
#include "cutils.h"
#include "BandList.h"

#include "Wsjtx_qt_helpers.hpp"
#include "WsjtxDecodesModel.hpp"
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


WsjtxFrame::WsjtxFrame(QWidget *parent) :
    QFrame(parent)
  , ui(new Ui::WsjtxFrame)
  , decodes_model_ {new DecodesModel()}
{
    ui->setupUi(this);

    bool autoEnabled;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTXAutoEnabled, autoEnabled );

    ui->autoSelectReplyFrame->setVisible(autoEnabled);

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = new TestDelegate(1.0, lcf/100.0);
    ui->decodes_table_view_->setItemDelegate( delegate);
    decodes_model_->delegate = delegate;
    decodes_model_->messages = &messages;

    ui->decodes_table_view_->setModel (decodes_model_);
    ui->decodes_table_view_->verticalHeader ()->hide ();
    ui->decodes_table_view_->hideColumn (dcId);
    ui->decodes_table_view_->hideColumn (dcSeq);
    ui->decodes_table_view_->hideColumn (dcPoints);
    ui->decodes_table_view_->hideColumn (dcBearing);
    ui->decodes_table_view_->hideColumn (dcDistance);
    ui->decodes_table_view_->hideColumn (dcFromCall);
    ui->decodes_table_view_->hideColumn (dcFromGrid);
    ui->decodes_table_view_->hideColumn (dcToCall);
    ui->decodes_table_view_->hideColumn (dcToGrid);

    ui->decodes_table_view_->horizontalHeader ()->setStretchLastSection (true);
    ui->decodes_table_view_->verticalHeader()->setMinimumSectionSize(1);

    ui->decodes_table_view_->resizeRowsToContents();

    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_log_ADIF, this, &WsjtxFrame::log_ADIF);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_add_client, this, &WsjtxFrame::add_client);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_remove_client, this, &WsjtxFrame::remove_client);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_remove_client, this, &WsjtxFrame::clear_decodes);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_decode_added, this, &WsjtxFrame::decode_added, Qt::QueuedConnection);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_clear_decodes, this, &WsjtxFrame::clear_decodes);//
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_update_status, this, &WsjtxFrame::update_status);

    // UI behaviour

    connect (ui->auto_off_button_, &QAbstractButton::clicked, [this] (bool /* checked */) {
        do_halt_tx (id_, true);
    });
    connect (ui->halt_tx_button_, &QAbstractButton::clicked, [this] (bool /* checked */) {
        do_halt_tx (id_, false);
    });

    // this to change - get the item, and use the message decode data
    connect (ui->decodes_table_view_, &QTableView::doubleClicked, this, &WsjtxFrame::do_reply);
}
WsjtxFrame::~WsjtxFrame()
{
    delete ui;
}
void WsjtxFrame::do_halt_tx(QString const& id, bool auto_only)
{
    ui->replyto_label->setText("");
    WsjtxServer::getWsjtxServer()->do_halt_tx(id, auto_only);
}
void WsjtxFrame::setContest(BaseContestLog *c)
{
    ct = c;
}

//void WsjtxFrame::log_qso (QString const& /*id*/, QDateTime time_off, QString const& dx_call
//                                           , QString const& dx_grid, Frequency dial_frequency, QString const& mode
//                                           , QString const& report_sent, QString const& report_received
//                                           , QString const& tx_power, QString const& comments
//                                           , QString const& name, QDateTime time_on, QString const& operator_call
//                                           , QString const& my_call, QString const& my_grid)
//{
//    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
//    if (ct != cc)
//        return;
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
//}
void WsjtxFrame::log_ADIF(QString const& id, QByteArray const& ADIF)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

    ui->replyto_label->setText("");
    int spoint = ct->ctList.count();
    if (! ADIFImport::doImportADIFString(dynamic_cast<LoggerContestLog *>(ct),  ADIF ))
    {
        MinosParameters::getMinosParameters() ->mshowMessage( "Failed to append ADIF from " + id );
    }
    ct->scanContest();
    ct->validateLoc();
    for ( int i = spoint; i != ct->ctList.count(); i++ )
    {
        QSharedPointer<BaseContact> bct = ct->pcontactAt(i);
        bct->commonSave(bct);
    }
    ct->commonSave( false );
    MinosLoggerEvents::SendAfterLogContact(ct);
    TSingleLogFrame * tslf = LogContainer ->findContest( ct );

    tslf->showQSOs();
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
void WsjtxFrame::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                , QString const& report, QString const& tx_mode, bool tx_enabled
                                , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, qint8 special_op_mode)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

    QColor fcolour = Qt::black;
    if (transmitting)
        fcolour = Qt::red;
    QColor dcolour = Qt::black;
    if (decoding)
        dcolour = Qt::blue;
    QColor tcolour = Qt::black;
    if (watchdog_timeout)
        tcolour = Qt::red;

    // pass to decodes model
    //  if (id == id_)
    //    {
    QString special;
    switch (special_op_mode)
    {
    case 0: special = "[Normal]"; break;
    case 1: special = "[NA VHF]"; break;
    case 2: special = "[EU VHF]"; break;
    case 3: special = "[FD]"; break;
    case 4: special = "[RTTY RU]"; break;
    case 5: special = "[Fox]"; break;
    case 6: special = "[Hound]"; break;
    default: special = "[Unknown]";
    }

    fast_mode_ = fast_mode;
    decoder.setMyCallGrid(de_call, de_grid);
    decodes_model_->de_call (de_call);
    decodes_model_->rx_df (rx_df);
    ui->de_label_->setText (de_call.size () >= 0 ? QString {"DE: %1%2"}.arg (de_call)
                                                   .arg (de_grid.size () ? '(' + de_grid + ')' : QString {}) : QString {});

    ui->specialOpMode->setText(special);
    ui->mode_label_->setText (HtmlFontColour(dcolour) + QString {"Mode: %1%2%3%4"}
                              .arg (mode)
                              .arg (sub_mode)
                              .arg (fast_mode && !mode.contains (QRegularExpression {R"(ISCAT|MSK144)"}) ? "fast" : "")
                              .arg (tx_mode.isEmpty () || tx_mode == mode ? "" : '(' + tx_mode + ')'));


    ui->frequency_label_->setText (HtmlFontColour(fcolour) + "QRG: " + Radio::pretty_frequency_MHz_string (f));

    BandList &blist = BandList::getBandList();
    BandInfo bi;
    double df = f;
    bool bandOK = blist.findBand(df, bi);
    if (bandOK)
    {
        QString cb = cc->band.getValue().trimmed();
        BandInfo cbi;
        bool bandOK = blist.findBand(cb, cbi);
        if (bandOK)
        {
            cb = cbi.uk;
        }
        if (cb != bi.uk)
        {
            QString mess = QString("<h1><b>Contest band %1 not the same as %2 band %3").arg(cb).arg(id).arg(bi.uk);
            ui->bandErrorLabel->setText(HtmlFontColour(Qt::red) + mess);
        }
        else
        {
            ui->bandErrorLabel->clear();
        }
    }

    ui->dx_label_->setText (dx_call.size () >= 0 ? QString {"DX: %1%2"}.arg (dx_call)
                                                   .arg (dx_grid.size () ? '(' + dx_grid + ')' : QString {}) : QString {});
    ui->rx_df_label_->setText (rx_df >= 0 ? QString {HtmlFontColour(tcolour) + "Rx: %1"}.arg (rx_df) : "");
    ui->tx_df_label_->setText (tx_df >= 0 ? QString {HtmlFontColour(tcolour) + "Tx: %1"}.arg (tx_df) : "");
    ui->report_label_->setText ("SNR: " + report);
    ui->auto_off_button_->setEnabled (tx_enabled);
    ui->halt_tx_button_->setEnabled (transmitting);

    static bool inDecode = false;

    if (decoding && !inDecode)
    {
        inDecode = true;
        decodeStartSize = messages.size();
    }
    if (inDecode && decoding == false)
    {
        inDecode = false;

        int decodeEndSize = messages.size();
        if (ui->autoSelectButton->isChecked() && decodeEndSize > decodeStartSize)
        {
            // iterate over the latest decodes, and select the best

            int bestOffset = -1;
            int bestPoints = -1;

            for (int i = decodeStartSize; i < decodeEndSize; i++)
            {
                decodeMessage &dc = messages[i];

                // NB that we get the decodes strongest first, so we shouldn't need to
                // select for the strongest decode for a station

                // we may end up needing to limit the message types
                // we can reply to.

                // we may also need to differentiate between VHF Eu and normal modes

                if (dc.mstage == emsCQ
                || dc.mstage == emsRRR
                || dc.mstage == ems73
                || (dc.mstage == emsGrid && dc.toCall.fullCall.getValue() == myCall)
                   )
                {
                    if ((!ui->snrCheckBox->isChecked() || dc.snr >= ui->snrSpinner->value() )
                            && (!ui->minPointsCheckBox->isChecked() || bestPoints >= ui->minPointsSpinner->value() )
                            && dc.points > bestPoints)
                    {
                        bestOffset = i;
                    }
                }
            }
            if (bestOffset > 0 )
            {
                reply(messages[bestOffset]);
            }
        }
    }
    //    }
}

void WsjtxFrame::decode_added (bool is_new, QString const& id, QTime time
                               , qint32 snr, float delta_time
                               , quint32 delta_frequency, QString const& mode
                               , QString const& message, bool low_confidence
                               , bool off_air)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

    decodeMessage dc = decoder.decode(id, time, snr, delta_time
                                      , delta_frequency, mode
                                      , message, low_confidence, off_air);

    // need to make use of the decode data stack, both here and in ::data
    if (!is_new)
    {
        trace("WsjtxFrame::decode_added - old message " + message);
        int target_row {-1};
        for (int row = 0; row < messages.size(); ++row)
        {
            decodeMessage &m = messages[row];
            if (m.id == id)
            {
                if (m.time == time
                        && m.snr == snr
                        && almost_equal( m.delta_time, delta_time, 2)
                        && m.delta_frequency == delta_frequency
                        && m.mode == mode
                        && m.low_confidence == low_confidence
                        && m.off_air == off_air
                        && m.message == message)
                {
                    return;   // message already present
                }
                if (time <= m.time)
                {
                    target_row = row; // last row with same time
                }
            }
        }
        if (target_row >= 0)
        {
            messages.insert(target_row + 1, dc);
        }
        else
            messages.push_back(dc);
    }
    else
    {
        trace("WsjtxFrame::decode_added - new message " + message);
        messages.push_back(dc);
    }

    decodes_model_->add_decode ();
    ui->decodes_table_view_->resizeRowToContents(messages.size() - 1);

    if (!columns_resized_)
    {
        ui->decodes_table_view_->resizeColumnsToContents ();
        columns_resized_ = true;
    }
    ui->decodes_table_view_->scrollToBottom ();
}
void WsjtxFrame::clear_decodes (QString const& /*client_id*/)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

    decodes_model_->removeRows(0, messages.size());
    messages.clear();

    columns_resized_ = false;
}
void WsjtxFrame::reply(decodeMessage &dc)
{
    ui->autoSelectButton->setChecked(false);
    ui->autoSelectButton->setArrowType(Qt::NoArrow);
    WsjtxServer::getWsjtxServer()->reply(dc.id, dc.time, dc.snr, dc.delta_time, dc.delta_frequency, dc.mode, dc.message, dc.low_confidence,  QApplication::keyboardModifiers () >> 24);
    ui->replyto_label->setText("Replying to: " + dc.message);
}
void WsjtxFrame::do_reply (QModelIndex index)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
        return;

    decodeMessage &dc = messages[index.row()];

    reply(dc);


}

void WsjtxFrame::on_autoSelectButton_toggled(bool c)
{
    if (!c)
    {
        //ui->autoSelectButton->setChecked(false);
        ui->autoSelectButton->setArrowType(Qt::NoArrow);
    }
    else
    {
        //ui->autoSelectButton->setChecked(true);
        ui->autoSelectButton->setArrowType(Qt::DownArrow);
    }
}
