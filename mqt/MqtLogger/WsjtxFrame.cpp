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
#include "WsjtxConfigureCQ.h"

#include "WsjtxFrame.h"
#include "ui_WsjtxFrame.h"

WsjtxFrame::WsjtxFrame(QWidget *parent) :
    QFrame(parent)
  , ui(new Ui::WsjtxFrame)
  , decodes_model_ {new DecodesModel()}
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 1);
    remove_client(QString());    // kill off the ratshit
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX1TestEnabled, showTest );
    ui->testButton->setVisible(showTest);

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX1AutoEnabled, autoEnabled );
    ui->autoSelectReplyFrame->setVisible(autoEnabled);

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate> (new HtmlDelegate(1.0, lcf/100.0));
    ui->decodes_table_view_->setItemDelegate( delegate.data());


    QSize ms = delegate->docSize("XX");
    ui->decodes_table_view_->verticalHeader()->setDefaultSectionSize(ms.height() *4/5);
    ui->decodes_table_view_->verticalHeader()->setMinimumSectionSize(10);

    ui->decodes_table_view_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->decodes_table_view_->horizontalHeader ()->setStretchLastSection (true);
    ui->decodes_table_view_->verticalHeader()->setMinimumSectionSize(1);

    ui->decodes_table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
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

    reloadColumns();

    ui->decodes_table_view_->hideColumn (dcId);
    ui->decodes_table_view_->hideColumn (dcDT);
    ui->decodes_table_view_->hideColumn (dcDF);
    ui->decodes_table_view_->hideColumn (dcMd);
    ui->decodes_table_view_->hideColumn (dcConfidence);
    ui->decodes_table_view_->hideColumn (dcLive);
    ui->decodes_table_view_->hideColumn (dcSeq);
    //ui->decodes_table_view_->hideColumn (dcPoints);
    //ui->decodes_table_view_->hideColumn (dcBearing);
    ui->decodes_table_view_->hideColumn (dcDistance);
    ui->decodes_table_view_->hideColumn (dcFromCall);
    //ui->decodes_table_view_->hideColumn (dcFromGrid);
    ui->decodes_table_view_->hideColumn (dcToCall);
    ui->decodes_table_view_->hideColumn (dcToGrid);


    if (autoEnabled)
        ui->decodes_table_view_->showColumn (dcBest);
    else
        ui->decodes_table_view_->hideColumn (dcBest);


    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_log_ADIF, this, &WsjtxFrame::log_ADIF);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_add_client, this, &WsjtxFrame::add_client);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_remove_client, this, &WsjtxFrame::remove_client);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_remove_client, this, &WsjtxFrame::decodes_cleared);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_decode_added, this, &WsjtxFrame::decode_added, Qt::QueuedConnection);
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_decodes_cleared, this, &WsjtxFrame::decodes_cleared);//
    connect (WsjtxServer::getWsjtxServer(), &WsjtxServer::do_update_status, this, &WsjtxFrame::update_status);

    // this to change - get the item, and use the message decode data
    connect (ui->decodes_table_view_, &QTableView::doubleClicked, this, &WsjtxFrame::do_reply);

    connect(&MinosLoggerEvents::mle, SIGNAL(doColumnChanges(BaseContestLog*)), this, SLOT(on_doColumnChanges(BaseContestLog*)));

    connect( ui->decodes_table_view_->horizontalHeader(), SIGNAL(sectionMoved(int, int , int)),
             this, SLOT( on_sectionMoved(int, int , int)));
    connect( ui->decodes_table_view_->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));

    restoreSplitters();
    connect(&MinosLoggerEvents::mle, SIGNAL(doSplitterChanges(BaseContestLog*)), this, SLOT(on_doSplitterChanges(BaseContestLog*)));

    getAllTxtEnd();
    getCQStrings();
}
WsjtxFrame::~WsjtxFrame()
{
    delete ui;
    delete decodes_model_;
}
void WsjtxFrame::getCQStrings()
{
    QString testCQ;
    QString nontestCQ;

    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX1TestCQ, testCQ );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX1NonTestCQ, nontestCQ );


    CSVToStringList(testCQ, testCQCalls);
    CSVToStringList(nontestCQ, nonTestCQCalls);
}
void WsjtxFrame::on_halt_tx_button__clicked()
{
    trace("do_halt_tx");
    ui->replyto_label->setText("");
    WsjtxServer::getWsjtxServer()->do_halt_tx(id_, false);
}

void WsjtxFrame::on_auto_off_button__clicked()
{
    trace("disable tx clicked");
    ui->replyto_label->setText("");
    WsjtxServer::getWsjtxServer()->do_halt_tx(id_, true);
}
void WsjtxFrame::on_clearDecodesButton_clicked()
{
    WsjtxServer::getWsjtxServer()->do_clear_decodes(id_, 2);    // 2 is "both windows"
}
void WsjtxFrame::on_clearLocalDecodesButton_clicked()
{
    decodes_cleared(id_);
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
//  << new QStandardItem {frequency_MHz_string (dial_frequency)}
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
    id_ = id;

    if (!bandOK)
        return;

    if (ct->isReadOnly())
        return;

    trace("WsjtxFrame::log_ADIF " + QString(ADIF));

    ui->replyto_label->setText("");
    int spoint = ct->ctList.count();

    // we will try to log the ADIF to ALL open contests; it will fail if the date/time or requency are wrong
    if (! ADIFImport::doImportADIFString(dynamic_cast<LoggerContestLog *>(ct),  ADIF ))
    {
        // failure may be because frequency or time don't match
        trace( "Failed to append ADIF from " + id );
        return;
    }
    ct->scanContest();
    ct->validateLoc();
    for ( int i = spoint; i < ct->ctList.count(); i++ )
    {
        QSharedPointer<BaseContact> bct = ct->pcontactAt(i);
        if (bct->loc.getLoc().isEmpty())
        {
            Callsign cs = bct->cs;
            const Locator loc = WsjtGetCallLoc(cs);
            bct->loc = loc;
            trace(QString("loc for %1 is empty; filling in with %2").arg(cs.getFullCall()).arg(loc.getLoc()));
        }
        bct->commonSave(bct);
    }
    ct->commonSave( false );
    MinosLoggerEvents::SendAfterLogContact(ct);
    TSingleLogFrame * tslf = LogContainer ->findContest( ct );

    tslf->showQSOs();
}
void WsjtxFrame::add_client (QString const& id, QString const& /*version*/, QString const& /*revision*/)
{
    id_ = id;
}

void WsjtxFrame::remove_client (QString const& /*id*/)
{
    ui->dx_label_->clear();
    ui->rx_df_label_->clear();
    ui->tx_df_label_->clear();
    ui->report_label_->clear();
    ui->auto_off_button_->setEnabled (false);
    ui->halt_tx_button_->setEnabled (false);
    ui->de_label_->clear();
    ui->frequency_label_->clear();
    ui->specialOpMode->clear();
    ui->mode_label_->clear();
    ui->bandErrorLabel->clear();
    ui->replyto_label->clear();
    if (ui->autoSelectButton->isChecked())
        ui->autoSelectButton->toggle();
    id_.clear();
}
class PointBonusMultSnr
{
    int points = -1;
    int bonus = 0;
    int mults = 0;
    int snr = -100;
public:
    PointBonusMultSnr()
    {

    }
    PointBonusMultSnr(decodeMessage &dc):points(dc.points), bonus(dc.bonus), mults(dc.mults), snr(dc.snr)
    {

    }
    bool operator>(PointBonusMultSnr &rhs)
    {
        if (mults > rhs.mults)
            return true;
        else if (mults == rhs.mults)
        {
            if (points + bonus > rhs.points + rhs.bonus)
                return true;
            else if (points + bonus == rhs.points + rhs.bonus)
            {
                if (snr > rhs.snr)
                    return true;
            }
        }
        return false;
    }
    bool operator==(PointBonusMultSnr &rhs)
    {
        if ((mults == rhs.mults) && (points + bonus == rhs.points + rhs.bonus) && snr == rhs.snr)
            return true;

        return false;
    }
};

void WsjtxFrame::process_decodes()
{
    if (!bandOK)
        return;

    if (autoEnabled)
    {
        int decodeEndSize = messages.size();
        int minpoints = ui->minPointsSpinner->value();
        if (!ui->minPointsCheckBox->isChecked())
            minpoints = 0;
        int minsnr =  ui->snrSpinner->value();
        if (!ui->snrCheckBox->isChecked())
            minsnr = -100;
        trace(QString("WsjtxFrame::process_decodes Checking decodes start %1 end %2 autoselect %3 minPoints %4 minSnr %5")
              .arg(decodeStartSize).arg(decodeEndSize).arg(ui->autoSelectButton->isChecked()).arg(minpoints).arg(minsnr));

        if (decodeEndSize > decodeStartSize)
        {
            // iterate over the latest decodes, and select the best

            int bestOffset = -1;
            PointBonusMultSnr bestPoints;

            bool autoReplyAllowed = !currentlyTransmitting;

            // first, look at messages against our transmit status
            // NB we go to emsNone when (!transmitting && !tx_enabled)
            // which when rr73/73 has been sent, at end of
            // update_status()

            if (currTxStage != emsNone)
            {
                for (int i = decodeStartSize; i < decodeEndSize; i++)
                {

                    decodeMessage &dc = messages[i];
                    if (dc.oldmsg)
                        continue;
                    if (dc.decodeInd[0] == "?")
                        continue;   // potentially bad decode

                    trace(QString("WsjtxFrame::process_decodes Checking against lastTx %1 stage %2 tocall %3 fromcall %4 callingCall %5 workingCall %6")
                          .arg(messages[i].message).arg(dc.getMStage()).arg(dc.toCall.getFullCall()).arg(dc.fromCall.getFullCall()).arg(callingCall).arg(workingCall));

                    // if we are calling CQ or RR73 or 73, and we are toCall, we have a set of candidates for best
                    // work these before lookng for others

                     PointBonusMultSnr pbv(dc);
                     bool toMyCall = (dc.toCall == decoder.getMyCall());
                     QString dcFromCall = dc.fromCall.getFullCall();

                     if (toMyCall)
                     {
                         if (currTxStage == emsCQ || currTxStage == emsRRR || currTxStage == ems73) // calling CQ or waiting for 73
                         {
                             // look for best candidate of those calling us - don't limit by snr or points
                             // If they are starting with Tx2, we can miss the loc - and so their score will
                             // be miniscule (probably 0)
                             // so start bestpoints at -1...
                             if ( pbv > bestPoints  )
                             {
                                 // which might not be the most profitable
                                 trace(QString("WsjtxFrame::process_decodes (lasttx) Candidate %1").arg(messages[i].message));
                                 bestOffset = i;
                                 bestPoints = pbv;

                                 autoReplyAllowed = true;
                             }
                             else
                             {
                                 trace(QString("WsjtxFrame::process_decodes (lasttx) NOT best %1").arg(messages[i].message));
                             }
                         }
                         else if (dcFromCall == workingCall || dcFromCall == callingCall)
                         {
                             // this is best, and WSJT-X should automatically respond
                             // so long as autoseq is set in WSJT-X

                             ui->decodes_table_view_->scrollToBottom ();
                             return;
                         }
                         else
                         {
                            // ignore it - random caller with us in mid QSO
                             continue;
                         }
                     }
                     else if (dcFromCall == callingCall || dcFromCall == workingCall)
                     {
                         // we are trying to work them, and they aren't working us - still CQ, or working someone else

                         // If they are calling CQ and we are "grid" we can carry on calling them
                        // don't kill tx unless there is a better option - using the general best search

                         // bestOffset should already be -1 unless there is someone else calling us
                         // in which case we will switch to them

                         trace(QString("WsjtxFrame::process_decodes (lasttx) stop response, look again"));
                         if (ui->autoRearmcb->isChecked())
                         {
                             ui->autoSelectButton->setChecked(reArmValue);
                         }
                         if (ui->autoSelectButton->isChecked())
                         {
                             on_halt_tx_button__clicked();          // kill the automatic sequencing
                         }
                         break;

                     }
                }
            }
             if (bestOffset < 0)
             {
                 // we don't already have a best
                 for (int i = decodeStartSize; i < decodeEndSize; i++)
                 {
                     decodeMessage &dc = messages[i];
                     if (dc.oldmsg)
                         continue;
                     if (dc.decodeInd[0] == "?")
                         continue;

                     trace(QString("WsjtxFrame::process_decodes Checking %1 stage %2 tocall %3 fromcall %4")
                           .arg(messages[i].message).arg(dc.getMStage()).arg(dc.toCall.getFullCall()).arg(dc.fromCall.getFullCall()));

                     if (dc.points <= 0)    // e.g. duplicate
                        continue;

                     PointBonusMultSnr pbv(dc);
                     bool toMyCall = (dc.toCall == decoder.getMyCall());
                     QString dcFromCall = dc.fromCall.getFullCall();

                     bool auto73 = ui->autosel73cb->isChecked();
                     if ((dc.mstage == emsCQ)   // CQ calls aren't "to" anyone
                             || (auto73 && (dc.mstage == ems73) && !toMyCall)
                             || (auto73 && (dc.mstage == emsRRR) && !toMyCall)
                             || ((dc.mstage == emsGrid) && toMyCall)
                       )
                    {
                         if (dc.mstage == emsCQ)
                         {
                             bool inTest = false;
                             switch (special_op_mode)
                             {
                             case 1:
                             case 2:
                                 inTest = true;
                                 break;

                             default:
                                 break;
                             }
                             if (inTest)
                             {
                                if (!testCQCalls.contains(dc.CQCall))
                                {
                                    trace(QString("WsjtxFrame::process_decodes %1 wrong CQ call").arg(messages[i].message));
                                    continue;
                                }
                             }
                             else
                             {
                                 if (!nonTestCQCalls.contains(dc.CQCall))
                                 {
                                     trace(QString("WsjtxFrame::process_decodes %1 wrong CQ call").arg(messages[i].message));
                                     continue;
                                 }

                             }
                         }
                        if ( dc.snr >= minsnr
                                && dc.points > minpoints
                                && pbv > bestPoints
                              )
                        {
                            trace(QString("WsjtxFrame::process_decodes Candidate %1").arg(messages[i].message));
                            bestOffset = i;
                            bestPoints = pbv;
                        }
                        else
                        {
                            trace(QString("WsjtxFrame::process_decodes NOT best %1").arg(messages[i].message));
                        }
                    }
                    else
                    {
                        trace(QString("WsjtxFrame::process_decodes NOT Candidate %1").arg(messages[i].message));
                    }
                }
            }

            if (bestOffset >= 0)
            {
                for (int i = decodeStartSize; i < decodeEndSize; i++)
                {
                    decodeMessage &dc = messages[i];
                    dc.best = (bestOffset == i);
                }
                trace("WsjtxFrame::process_decodes best decode is " + messages[bestOffset].message);
            }

            if (autoReplyAllowed && ui->autoSelectButton->isChecked() && bestOffset >= 0 )
            {
                trace("WsjtxFrame::process_decodes auto replying to " + messages[bestOffset].message);
                messages[bestOffset].autoresp = true;
                reply(messages[bestOffset]);

                // we are assuming that autoseq is enabled, call 1st isn't
                // but we can't enforce either
                // things may be messy if we are not set this way
            }
            emit decodes_model_->dataChanged(decodes_model_->index(decodeStartSize, dcBest), decodes_model_->index(decodeEndSize, dcBest));
        }
    }

    ui->decodes_table_view_->scrollToBottom ();
}
void WsjtxFrame::getAllTxtEnd()
{
    QString fname = WsjtxServer::getDataPath() + "/ALL.TXT";
    alltxt.setFileName(fname);

    if (!alltxt.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QString ebuff = tr( "Failed to open %1" ).arg(fname);
        trace(ebuff);
        return;
    }
    alltxt.seek(alltxt.size());
    alltxtstr.setDevice(&alltxt);
    while (!alltxtstr.atEnd())
    {

      alltxtstr.readLine(255);
    }
}
decodeMessage *WsjtxFrame::parse_tx_message(QString atline, bool fromScrape)
{
    // 200425_110345    50.313 Tx FT8      0  0.0 1500 CQ G0GJV IO91

    // 200424_160138     7.048 Rx FT4    -10  0.0  817 CQ YO4NF KN44
    // 200424_160138     7.048 Rx FT4      2 -0.1 1399 AM4WARD <DL9DAJ> 73
    // 200424_160138     7.048 Rx FT4     -8  0.2 2798 CQ R9SDO LO91
    // 200424_160138     7.048 Rx FT4     -9  0.0  292 CQ ON5ZZ JO11                       ? a1
    // 200424_160145     7.048 Rx FT4      1 -0.0  504 R9SDO R2ASY +05

    // "? a1" is about a priori decodes and their possible validity

    // date-time rigfreq txrx mode s/n dt df message

    QString id = "WSJTX";
    QTime time;
    qint32 snr = 0;
    float delta_time = 0;
    quint32 delta_frequency = 0;
    QString mode;
    QString message;
    bool low_confidence = false;    // we sent it, after all

    if (fromScrape)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList sl = atline.trimmed().split(' ', Qt::SkipEmptyParts);
#else
        QStringList sl = atline.trimmed().split(' ', QString::SkipEmptyParts);
#endif
        if (sl[2] != "Tx")
            return nullptr;

        //trace("Tx scraped: " + atline);
        time = QDateTime::fromString(sl[0], "yyMMdd_HHmmss").time();
        //double rigfreq = sl[1].toDouble();
        // sl[2] == Tx
        mode = sl[3];
        snr = 0;    //sl[4]
        delta_time = 0.0;   //sl[5]
        delta_frequency = sl[6].toUInt();

        for (int i = 0; i < 7; i++)
        {
            sl.removeFirst();
        }
        message = sl.join(' ');

    }
    else
    {
        message = atline;
    }
    decodeMessage dc = decoder.decode(id, eTX, time, snr, delta_time
                                    , delta_frequency, mode
                                    , message, low_confidence, true);

    trace(QString("WsjtxFrame::scrapeAllTxt - time %1 stage %2 %3")
          .arg(time.toString("HH:mm:ss")).arg(dc.getMStage(), atline));

    currTxStage = dc.mstage;

    messages.push_back(dc);

    decodeMessage *last =  &messages.last();
    return last;
}

decodeMessage *WsjtxFrame::scrapeAllTxt()
{
    decodeMessage *last = nullptr;
    if (alltxt.isOpen())
    {
        while (!alltxtstr.atEnd())
        {

          QString atline = alltxtstr.readLine(255);

          if (!bandOK)
              continue;

          // now we need to parse for transmissions
            last = parse_tx_message(atline, true); // which normally happens in process_decodes

            // process_decodes(); - not wanted, we've just started transmitting; previous process_decodes should have stopped us!
        }
    }
    return last;
}
void WsjtxFrame::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                , QString const& report, QString const& tx_mode, bool tx_enabled
                                , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, quint8 so_mode
                                , quint32 frequency_tolerance, quint32 tr_period
                                , QString const& configuration_name, QString const& tx_message)
{
//    MinosParameters *mp = MinosParameters::getMinosParameters();
//    if (!mp)
//        return;
//    BaseContestLog * cc = mp ->getCurrentContest();
//    if (ct != cc || cc == nullptr)
//        return;

    special_op_mode = so_mode;
    // protected contests aren't interesting
    if (!ct || ct->isReadOnly())
        return;

    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    double df = f;
    bandOK = blist.findBand(df, bi);
    if (bandOK)
    {
        QString cb = ct->contestBands.getValue().trimmed();
        if (cb == allHF)
        {
            cb = bi->uk;
            ct->setCurrentBand(cb);
        }
        else
        {
            QSharedPointer<BandInfo>  cbi;
            bool cbandOK = blist.findBand(cb, cbi);
            if (cbandOK)
            {
                cb = cbi->uk;
            }
            if (cb != bi->uk)
            {
                QString mess = tr("<h1><b>Contest band %1 not the same as %2 band %3").arg(cb).arg(id).arg(bi->uk);
                ui->bandErrorLabel->setText(HtmlFontColour(Qt::red) + mess);
            }
            else
            {
                ui->bandErrorLabel->clear();
            }
        }
    }

    // if the band is wrong, then we shouldn't be looking at it; certainly not for auto...
    // it should be going to the contest that IS on the correct band, even if it isn't
    // the current contest

    // there should NEVER be two simultaneous contest on the same band. Although Ken may test it...

    if (!bandOK)
    {
        scrapeAllTxt(); // move to EOF withut doing anything
        return;
    }

    id_ = id;

    trace(QString("WsjtxFrame::update_status dx_call %1 dx_grid %2 transmitting %3 decoding %4 tx_enabled %5")
          .arg(dx_call).arg(dx_grid).arg(transmitting).arg(decoding).arg(tx_enabled));

    decodeMessage *lastTx = nullptr;
    if (transmitting && !currentlyTransmitting)
    {
        // try scraping the transmissions from the all.txt file
        if (tx_message.isEmpty())
        {
            lastTx = scrapeAllTxt();
        }
        else
        {
            //lastTx = parse_tx_message( tx_message, false);

            decodeMessage dc = decoder.decode(id, eTX, QTime::currentTime(), 0, 0
                                            , 0, mode
                                            , tx_message, false, true);

            trace(QString("WsjtxFrame::parse_tx_message - stage %1 %2")
                  .arg(dc.getMStage(), tx_message));

            currTxStage = dc.mstage;

            messages.push_back(dc);

            lastTx =  &messages.last();
        }
        decodes_model_->add_decode ();
        ui->decodes_table_view_->scrollToBottom ();
    }
    currentlyDecoding = decoding;
    currentlyTransmitting = transmitting;

    if (!transmitting && !tx_enabled)
    {
        currTxStage = emsNone;
        callingCall.clear();
        workingCall.clear();
        lastTx = nullptr;
    }
    else if (lastTx)
    {
        currTxStage = lastTx->mstage;
        callingCall.clear();
        workingCall.clear();
        // we transmitted in the last period
        // set calling call - if we are
        // set working call - if we are
        switch (lastTx->mstage)
        {
        case emsNone:
            // shouldn't happen...
        case emsCQ:
            // repliable to "from"
            // CQ K1ABC FN42
            // CQ DX K1ABC FN42
            // CQ TEST G4ABC/P IO91

            if (ui->autoRearmcb->isChecked())
            {
                ui->autoSelectButton->setChecked(reArmValue);
            }
            break;
        case ems73:
            // after this may still get far end RRR or RR73
            // repliable to "from"
            // K1ABC G0XYZ 73
            // G4ABC/P PA9XYZ 73
        case emsFree:
            // no calls involved
            break;

        case emsGrid:
            // grid is sl[2] is 4 fig loc
            // K1ABC G0XYZ IO91
            // G4ABC/P PA9XYZ JO22
            callingCall = dx_call;
            break;

        case emsDb:
            // db is aything else? or it may be a free text message
            //G0XYZ K1ABC –19
            //PA9XYZ 590003 IO91NP
        case emsRplusDb:
            // repliable to "from" - but may have to wait for 73
            // K1ABC G0XYZ R-22
            // G4ABC/P R 570007 JO22DB
        case emsRplusGrid:
            // similar to emsRplusDb
        case emsRplusDbGrid:
        case emsDbGrid:
        case emsRRR:
            // repliable to "from"
            // G0XYZ K1ABC RR73
            // PA9XYZ G4ABC/P RR73
            workingCall = dx_call;
            break;
        }
        trace(QString("WsjtxFrame::update_status last tx stage %1 calling <%2> working <%3>").arg(lastTx->getMStage()).arg(callingCall).arg(workingCall));
    }

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
                              .arg (tx_mode.isEmpty () || tx_mode == mode ? QString() : '(' + tx_mode + ')'));


    ui->frequency_label_->setText (HtmlFontColour(fcolour) + "QRG: " + f.pretty_frequency_MHz_string());

    ui->dx_label_->setText (dx_call.size () >= 0 ? QString {"DX: %1%2"}.arg (dx_call)
                                                   .arg (dx_grid.size () ? '(' + dx_grid + ')' : QString {}) : QString {});
    ui->rx_df_label_->setText (rx_df >= 0 ? QString {HtmlFontColour(tcolour) + "Rx: %1"}.arg (rx_df) : "");
    ui->tx_df_label_->setText (tx_df >= 0 ? QString {HtmlFontColour(tcolour) + "Tx: %1"}.arg (tx_df) : "");
    ui->report_label_->setText ("SNR: " + report);
    ui->auto_off_button_->setEnabled (tx_enabled);
    ui->halt_tx_button_->setEnabled (transmitting);

    if (decoding && !inDecode)
    {
        inDecode = true;
        decodeStartSize = messages.size();
    }
    if (inDecode && decoding == false)
    {
        inDecode = false;

        process_decodes();
    }
}

void WsjtxFrame::decode_added (bool is_new, QString const& id, QTime time
                               , qint32 snr, float delta_time
                               , quint32 delta_frequency, QString const& mode
                               , QString const& message, bool low_confidence
                               , bool off_air)
{
    if (!bandOK)
        return;
    if (!ct || ct->isReadOnly())
        return;

    id_ = id;

    decodeMessage dc = decoder.decode(id, eRX, time, snr, delta_time
                                      , delta_frequency, mode
                                      , message, low_confidence, off_air);

    if (lastTime != dc.time)
    {
        lastcol++;
        lastcol %= 4;
        lastTime = dc.time;
    }

    dc.colOffset = lastcol;
    dc.oldmsg = !is_new;

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

    if (!currentlyDecoding && !currentlyTransmitting && !inDecode && is_new)
    {
        // we can get "free standing" decodes, which we should process for best/auto
        process_decodes();
    }

}
void WsjtxFrame::decodes_cleared (QString const& client_id)
{
    // don't check for contest - clear is across all contests

    id_ = client_id;
    decodes_model_->clear();
}
void WsjtxFrame::reply(decodeMessage &dc)
{
    reArmValue = ui->autoSelectButton->isChecked();

    ui->autoSelectButton->setChecked(false);
    //ui->autoSelectButton->setArrowType(Qt::NoArrow);
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
        //ui->autoSelectButton->setArrowType(Qt::NoArrow);
        trace("WsjtxFrame autoselect off");
    }
    else
    {
        //ui->autoSelectButton->setArrowType(Qt::DownArrow);
        trace("WsjtxFrame autoselect on");
    }
}

void WsjtxFrame::on_testButton_clicked()
{

//    void WsjtxServer::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
//                                      , QString const& report, QString const& tx_mode, bool tx_enabled
//                                      , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
//                                      , QString const& de_call, QString const& de_grid, QString const& dx_grid
//                                      , bool watchdog_timeout, QString const& sub_mode, bool fast_mode
//                                     , quint8 special_op_mode, quint32 frequency_tolerance, quint32 tr_period
//                                     , QString const& configuration_name, QString const& tx_message)


            update_status ("test", Frequency(14070060), "FT8", "","0", "FT8", false, false, true, 0, 0
                                    , "G0GJV", "IO91", "JO01"
                                    , false, "", false, 0, 0, 0, "", "");


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

            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0XYZ K1ABC -19", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "<G4ABC> <PA9XYZ> R 580071 JO22DB", false, true);

            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0GJV M0GXZ IO92", false, true);
            decode_added(true, "test", now, -2, 0, 0, "FT8", "G0GJV G8KWX -19", false, true);
            decode_added(true, "test", now, -1, 0, 0, "FT8", "G0GJV G3ZPB IO91", false, true);
            decode_added(true, "test", now, -1, 0, 0, "FT8", "G0GJV G3ZPB RR73", false, true);
            decode_added(true, "test", now, -19, 0, 0, "FT8", "<GB1945PJ> SP9DEM JO90", false, true);
            decode_added(true, "test", now, -19, 0, 0, "FT8", "<...> RW3SK KO94", false, true);


            // Normal
            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ K1ABC FN42            a1", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC G0XYZ IO91", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0XYZ K1ABC -19", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC G0XYZ R-22", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0XYZ K1ABC RR73", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC G0XYZ 73", false, true);


            // EU VHF Contest

            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ TEST G4ABC/P IO91", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G4ABC/P PA9XYZ JO22", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "PA9XYZ 590003 IO91NP", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G4ABC/P R 570007 JO22DB", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "PA9XYZ G4ABC/P RR73", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G4ABC/P PA9XYZ 73", false, true);

            //And in 2.2 (both callsigns, hashed)
            //(Either callsign (or both) may have /P appended.)

            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ TEST G4ABC IO91", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G4ABC PA9XYZ JO22", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "<PA9XYZ> <G4ABC> 570123 IO91NP", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "<G4ABC> <PA9XYZ> R 580071 JO22DB", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "PA9XYZ G4ABC RR73", false, true);


            // NA VHF Contest
            //(Either callsign (or both) may have /R appended. You can use RR73 in place of RRR, and the final 73 is optional)

            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ TEST K1ABC FN42", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC W9XYZ EN37", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "W9XYZ K1ABC R FN42", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC W9XYZ RR73", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "W9XYZ K1ABC 73", false, true);

            update_status ("test", Frequency(14070060), "FT8", "","0", "FT8", false, false, false, 0, 0
                                    , "G0GJV", "IO91", "JO01"
                                    , false, "", false, 0, 0, 0, "", "");

}


void WsjtxFrame::on_splitter_splitterMoved(int /*pos*/, int /*index*/)
{

    QByteArray state = ui->splitter->saveState();
    QSettings settings;
    settings.setValue("Splitters/WsjtxFrame/state/", state);

    MinosLoggerEvents::SendSplittersChanged();
}


void WsjtxFrame::restoreSplitters()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("Splitters/WsjtxFrame/state/").toByteArray();
    ui->splitter->restoreState(state);
}
void WsjtxFrame::on_doSplitterChanges(BaseContestLog *b)
{
    if (b == ct)
    {
        restoreSplitters();
    }
}
void WsjtxFrame::saveAllColumnWidthsAndPositions()
{
    if (!suppressSaveColumns)
    {
        QSettings settings;
        QByteArray state;

        state = ui->decodes_table_view_->horizontalHeader()->saveState();
        settings.setValue("decodes_table_view_/state", state);

        //And we need to send this out to all other instances

        MinosLoggerEvents::SendColumnsChanged();

    }
}
void WsjtxFrame::reloadColumns()
{
    QSettings settings;
    QByteArray state = settings.value("decodes_table_view_/state").toByteArray();
    if (state.size())
    {
        suppressSaveColumns = true;
        // this will fire signals, so... don't save at the same time
        ui->decodes_table_view_->horizontalHeader()->restoreState(state);
        suppressSaveColumns = false;
    }
}
void WsjtxFrame::on_doColumnChanges(BaseContestLog *b)
{
    if (b == ct)
    {
        reloadColumns();
    }
}
void WsjtxFrame:: on_sectionMoved(int /*logicalIndex*/, int /*oldVisualIndex*/, int /*newVisualIndex*/)
{
    saveAllColumnWidthsAndPositions();
}

void WsjtxFrame::on_sectionResized(int /*logicalIndex*/, int /*oldSize*/, int /*newSize*/)
{
    saveAllColumnWidthsAndPositions();
}

void WsjtxFrame::on_configCQButton_clicked()
{
    WsjtxConfigureCQ wccq(this);

    wccq.exec();

    getCQStrings();
}
