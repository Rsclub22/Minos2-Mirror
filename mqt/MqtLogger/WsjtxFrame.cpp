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

   // ui->testButton->setVisible(false);

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTXAutoEnabled, autoEnabled );

    ui->autoSelectReplyFrame->setVisible(autoEnabled);

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = new TestDelegate(1.0, lcf/100.0);
    ui->decodes_table_view_->setItemDelegate( delegate);
    decodes_model_->delegate = delegate;
    decodes_model_->messages = &messages;

//    enum DecodeColumns
//    {
//        dcId,
//        dcTime, dcSnr, dcDT, dcDF, dcMd, dcConfidence, dcLive,
//        dcSeq, dcPoints, dcBearing, dcDistance,
//        dcFromCall, dcFromGrid, dcToCall, dcToGrid,
//        dcBest,
//        dcMessage, dcMaxVal
//    };

    ui->decodes_table_view_->setModel (decodes_model_);
    ui->decodes_table_view_->verticalHeader ()->hide ();
    ui->decodes_table_view_->hideColumn (dcId);
    ui->decodes_table_view_->hideColumn (dcDT);
    ui->decodes_table_view_->hideColumn (dcMd);
    ui->decodes_table_view_->hideColumn (dcConfidence);
    ui->decodes_table_view_->hideColumn (dcLive);
    ui->decodes_table_view_->hideColumn (dcSeq);
    //ui->decodes_table_view_->hideColumn (dcPoints);
    //ui->decodes_table_view_->hideColumn (dcBearing);
    ui->decodes_table_view_->hideColumn (dcDistance);
    ui->decodes_table_view_->hideColumn (dcFromCall);
    ui->decodes_table_view_->hideColumn (dcFromGrid);
    ui->decodes_table_view_->hideColumn (dcToCall);
    ui->decodes_table_view_->hideColumn (dcToGrid);


    if (!autoEnabled)
        ui->decodes_table_view_->hideColumn (dcBest);

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
    if (ct != cc || ct->isProtected())
        return;

    trace("WsjtxFrame::log_ADIF " + QString(ADIF));

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
        return;
    id_.clear();
}
void WsjtxFrame::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                , QString const& report, QString const& tx_mode, bool tx_enabled
                                , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, qint8 special_op_mode)
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc || cc == nullptr)
        return;

    trace(QString("WsjtxFrame::update_status dx_call %1 dx_grid %2 transmitting %3 decoding %4 tx_enabled %5")
          .arg(dx_call).arg(dx_grid).arg(transmitting).arg(decoding).arg(tx_enabled));
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

    if (autoEnabled)
    {
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
            trace(QString("WsjtxFrame::update_status Checking decodes start %1 end %2").arg(decodeStartSize).arg(decodeEndSize));

            if (decodeEndSize > decodeStartSize)
            {
                // iterate over the latest decodes, and select the best

                int bestOffset = -1;
                int bestPoints = -1;
                QString bestCs;
                int currSn = -100;

                int minpoints = ui->minPointsSpinner->value();
                if (!ui->minPointsCheckBox->isChecked())
                    minpoints = 0;
                int minsnr =  ui->snrSpinner->value();
                if (!ui->snrCheckBox->isChecked())
                    minsnr = -100;

                for (int i = decodeStartSize; i < decodeEndSize; i++)
                {
                    decodeMessage &dc = messages[i];

                    trace(QString("WsjtxFrame::update_status Checking %1 stage %2 tocall %3 fromcall %4")
                          .arg(messages[i].message).arg(dc.getMStage()).arg(dc.toCall.fullCall.getValue()).arg(dc.fromCall.fullCall.getValue()));

                    if (dx_call == dc.fromCall.fullCall.getValue())
                        continue;

                    if (dc.points <= 0)
                        continue;

//                    QString tc = dc.toCall.fullCall.getValue();
//                    QString mc = decoder.getMyCall().fullCall.getValue();
//                    QString st = dc.getMStage();

                    if (dc.mstage == emsCQ
                    || (dc.mstage == emsGrid && dc.toCall == decoder.getMyCall())
                       )
                    {

                        if ( dc.snr >= minsnr
                                && dc.points > minpoints
                                && dc.points > bestPoints
                              )
                        {
                            trace(QString("WsjtxFrame::update_status Candidate %1").arg(messages[i].message));
                            bestOffset = i;
                            bestPoints = dc.points;
                            bestCs = dc.fromCall.fullCall.getValue();
                            currSn = dc.snr;
                        }
                        else
                        {
                            if (bestOffset >= 0
                                && dc.fromCall.fullCall.getValue() == bestCs
                                && dc.snr > currSn
                                )
                            {
                                trace(QString("WsjtxFrame::update_status Candidate - CS already seen %1").arg(messages[i].message));
                                bestOffset = i;
                                currSn = dc.snr;
                            }
                            else
                            {
                                trace(QString("WsjtxFrame::update_status NOT best %1").arg(messages[i].message));

                            }
                        }
                    }
                    else
                    {
                        trace(QString("WsjtxFrame::update_status NOT Candidate %1").arg(messages[i].message));
                    }
                }
                for (int i = decodeStartSize; i < decodeEndSize; i++)
                {
                    decodeMessage &dc = messages[i];
                    dc.best = (bestOffset == i);
                }
                if (bestOffset >= 0)
                    trace("WsjtxFrame::update_status best decode is " + messages[bestOffset].message);

                emit decodes_model_->dataChanged(decodes_model_->index(decodeStartSize, dcBest), decodes_model_->index(decodeEndSize, dcBest));
                if (ui->autoSelectButton->isChecked() && bestOffset >= 0 )
                {
                    trace("WsjtxFrame::update_status auto replying to " + messages[bestOffset].message);
                    messages[bestOffset].autoresp = true;
                    reply(messages[bestOffset]);
                }
            }
        }
    }
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
        trace(QString("WsjtxFrame::decode_added - %1 old message %2").arg(time.toString("HH:mm:ss")).arg( message));
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
        trace(QString("WsjtxFrame::decode_added - %1 new message %2 stage %3 points %4 snr %5").arg(time.toString("HH:mm:ss")).arg(message).arg(dc.getMStage()).arg(dc.points).arg(dc.snr));
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
    // don't check for contest - clear is across all contests

    decodes_model_->clear();

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

    trace(QString("WsjtxFrame::do_reply on %1").arg(dc.message));

    reply(dc);


}

void WsjtxFrame::on_autoSelectButton_toggled(bool c)
{
    if (!c)
    {
        //ui->autoSelectButton->setChecked(false);
        ui->autoSelectButton->setArrowType(Qt::NoArrow);
        trace("WsjtxFrame autoselect off");
    }
    else
    {
        //ui->autoSelectButton->setChecked(true);
        ui->autoSelectButton->setArrowType(Qt::DownArrow);
        trace("WsjtxFrame autoselect on");
    }
}

void WsjtxFrame::on_testButton_clicked()
{

//update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
//        , QString const& report, QString const& tx_mode, bool tx_enabled
//        , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
//        , QString const& de_call, QString const& de_grid, QString const& dx_grid
//        , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, qint8 special_op_mode)


            update_status ("test", 12345, "FT8", "","0", "FT8", false, false, true, 0, 0
                                    , "G0GJV", "IO91", "JO01"
                                    , false, "", false, 0);


//void WsjtxFrame::decode_added (bool is_new, QString const& id, QTime time
//       , qint32 snr, float delta_time
//       , quint32 delta_frequency, QString const& mode
//       , QString const& message, bool low_confidence
//       , bool off_air)

//19:30:14.516 WsjtxFrame::decode_added - 18:30:00 new message CQ G4FTC IO91 stage CQ points 0 snr 13
//19:30:14.580 WsjtxFrame::decode_added - 18:30:00 new message G0GJV M0GXZ IO92 stage Grid points 112 snr -24
//19:30:14.690 WsjtxFrame::decode_added - 18:30:00 new message G0GJV G8KWX IO91 stage Grid points 50 snr -15
//19:30:14.733 WsjtxFrame::decode_added - 18:30:00 new message G0GJV G3ZPB IO91 stage Grid points 50 snr -15
            QTime now = QTime::currentTime();
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0GJV M0GXZ IO92", false, true);
            decode_added(true, "test", now, -2, 0, 0, "FT8", "G0GJV G8KWX -19", false, true);
            decode_added(true, "test", now, -1, 0, 0, "FT8", "G0GJV G3ZPB IO91", false, true);

            update_status ("test", 12345, "FT8", "","0", "FT8", false, false, false, 0, 0
                                    , "G0GJV", "IO91", "JO01"
                                    , false, "", false, 0);

}
