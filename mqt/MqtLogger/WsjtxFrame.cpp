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
    delegate = QSharedPointer<HtmlDelegate> (new TestDelegate(1.0, lcf/100.0));
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


    if (!autoEnabled)
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

    reloadColumns();

    connect(&MinosLoggerEvents::mle, SIGNAL(doColumnChanges(BaseContestLog*)), this, SLOT(on_doColumnChanges(BaseContestLog*)));

    connect( ui->decodes_table_view_->horizontalHeader(), SIGNAL(sectionMoved(int, int , int)),
             this, SLOT( on_sectionMoved(int, int , int)));
    connect( ui->decodes_table_view_->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));

    restoreSplitters();
    connect(&MinosLoggerEvents::mle, SIGNAL(doSplitterChanges(BaseContestLog*)), this, SLOT(on_doSplitterChanges(BaseContestLog*)));

    getAllTxtEnd();
}
WsjtxFrame::~WsjtxFrame()
{
    delete ui;
    delete decodes_model_;
}

void WsjtxFrame::on_halt_tx_button__clicked()
{
    ui->replyto_label->setText("");
    WsjtxServer::getWsjtxServer()->do_halt_tx(id_, true);
}

void WsjtxFrame::on_auto_off_button__clicked()
{
    ui->replyto_label->setText("");
    WsjtxServer::getWsjtxServer()->do_halt_tx(id_, false);
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
    id_ = id;

    if (ct->isProtected())
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
        if (bct->loc.loc.getValue().isEmpty())
        {
            Callsign cs = bct->cs;
            const Locator loc = WsjtGetCallLoc(cs);
            bct->loc.loc.setValue(loc.loc);
            trace(QString("loc for %1 is empty; filling in with %2").arg(cs.fullCall.getValue()).arg(loc.loc.getValue()));
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
class PointBonusMult
{
    int points = 0;
    int bonus = 0;
    int mults = 0;
public:
    PointBonusMult()
    {

    }
    PointBonusMult(decodeMessage &dc):points(dc.points), bonus(dc.bonus), mults(dc.mults)
    {

    }
    bool operator>(PointBonusMult &rhs)
    {
        if (mults > rhs.mults)
            return true;
        if (points + bonus > rhs.points + rhs.bonus)
            return true;

        return false;
    }
};

void WsjtxFrame::process_decodes()
{
    int decodeEndSize = messages.size();
    if (autoEnabled)
    {
        trace(QString("WsjtxFrame::process_decodes Checking decodes start %1 end %2").arg(decodeStartSize).arg(decodeEndSize));

        if (decodeEndSize > decodeStartSize)
        {
            // iterate over the latest decodes, and select the best

            int bestOffset = -1;
            PointBonusMult bestPoints;
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
                if (currTxStage == emsNone)
                {
                    // we aren't transmitting, so don't look for responders
                    break;
                }

                decodeMessage &dc = messages[i];
                if (dc.oldmsg)
                    continue;

                trace(QString("WsjtxFrame::process_decodes Checking against lastTx %1 stage %2 tocall %3 fromcall %4")
                      .arg(messages[i].message).arg(dc.getMStage()).arg(dc.toCall.fullCall.getValue()).arg(dc.fromCall.fullCall.getValue()));


                // NB we need to "re-arm" auto or we can't work someone else

                // Is auto now a tristate? active, inactive, able to be re-activated (or two checkboxes to make it clearer)
                // Does this go beyond permissable automation?

                // if we are calling CQ or RR73, and we are toCall, we have a set of candidates for best
                // work these before lookng for others

                 if (dc.points <= 0)    // e.g. duplicate
                    continue;

                 PointBonusMult pbv(dc);
                 bool toMyCall = (dc.toCall == decoder.getMyCall());
                 QString dcFromCall = dc.fromCall.fullCall.getValue();

                 if (currTxStage == emsRRR && toMyCall && dcFromCall == workingCall)
                 {
                     // this is best, and WSJT-X should automatically respond
                 }
                 else if (toMyCall && (currTxStage == emsCQ || currTxStage == emsRRR))
                 {
                     // look for best candidate of those calling us - don't limit by snr or points
                     if ( pbv > bestPoints  )
                     {
                         trace(QString("WsjtxFrame::process_decodes (lasttx) Candidate %1").arg(messages[i].message));
                         bestOffset = i;
                         bestPoints = pbv;
                         bestCs = dcFromCall;
                         currSn = dc.snr;
                     }
                     else
                     {
                         if (bestOffset >= 0
                             && dcFromCall == bestCs
                             && dc.snr > currSn
                             )
                         {
                             trace(QString("WsjtxFrame::process_decodes (lasttx) Candidate - CS already seen %1").arg(messages[i].message));
                             bestOffset = i;
                             currSn = dc.snr;
                         }
                         else
                         {
                             trace(QString("WsjtxFrame::process_decodes (lasttx) NOT best %1").arg(messages[i].message));

                         }
                     }
                 }
                 else if ((dcFromCall == callingCall || dcFromCall == workingCall) && !toMyCall)
                 {
                     // we are trying to work them, and they aren't working us
                     if (dc.mstage == emsCQ && currTxStage == emsGrid)
                     {
                        // If they are calling CQ and we are "grid" we can carry on calling them
                        // don't kill tx unless there is a better option - using the general best search
                     }
                     if (currTxStage > emsGrid)
                     {
                         // If our stage is later than grid, (their stage is irrelevant)
                         // someone else appears to be in QSO with them - stop transmission
                         on_halt_tx_button__clicked();
                         continue;  // this won't be an option for "best"!
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

                     trace(QString("WsjtxFrame::process_decodes Checking %1 stage %2 tocall %3 fromcall %4")
                           .arg(messages[i].message).arg(dc.getMStage()).arg(dc.toCall.fullCall.getValue()).arg(dc.fromCall.fullCall.getValue()));

                     if (dc.points <= 0)    // e.g. duplicate
                        continue;

                     PointBonusMult pbv(dc);
                     bool toMyCall = (dc.toCall == decoder.getMyCall());
                     QString dcFromCall = dc.fromCall.fullCall.getValue();

                     bool auto73 = ui->autosel73cb->isChecked();
                     if (dc.mstage == emsCQ
                             || (auto73 && dc.mstage == ems73 && !toMyCall)
                             || (auto73 && dc.mstage == emsRRR && !toMyCall)
                             || (dc.mstage == emsGrid && toMyCall)
                       )
                    {
                        if ( dc.snr >= minsnr
                                && dc.points > minpoints
                                && pbv > bestPoints
                              )
                        {
                            trace(QString("WsjtxFrame::process_decodes Candidate %1").arg(messages[i].message));
                            bestOffset = i;
                            bestPoints = pbv;
                            bestCs = dcFromCall;
                            currSn = dc.snr;
                        }
                        else
                        {
                            if (bestOffset >= 0
                                && dcFromCall == bestCs
                                && dc.snr > currSn
                                )
                            {
                                trace(QString("WsjtxFrame::process_decodes Candidate - CS already seen %1").arg(messages[i].message));
                                bestOffset = i;
                                currSn = dc.snr;
                            }
                            else
                            {
                                trace(QString("WsjtxFrame::process_decodes NOT best %1").arg(messages[i].message));

                            }
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

            emit decodes_model_->dataChanged(decodes_model_->index(decodeStartSize, dcBest), decodes_model_->index(decodeEndSize, dcBest));
            if (!currentlyTransmitting && ui->autoSelectButton->isChecked() && bestOffset >= 0 )
            {
                trace("WsjtxFrame::process_decodes auto replying to " + messages[bestOffset].message);
                messages[bestOffset].autoresp = true;
                reply(messages[bestOffset]);
            }
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
decodeMessage *WsjtxFrame::scrapeAllTxt()
{
    trace("WsjtxFrame::scrapeAllTxt()");
    decodeMessage *last = nullptr;
    if (alltxt.isOpen())
    {
        while (!alltxtstr.atEnd())
        {

          QString atline = alltxtstr.readLine(255);
          // now we need to parse for transmissions
// 200425_110345    50.313 Tx FT8      0  0.0 1500 CQ G0GJV IO91

// 200424_160138     7.048 Rx FT4    -10  0.0  817 CQ YO4NF KN44
// 200424_160138     7.048 Rx FT4      2 -0.1 1399 AM4WARD <DL9DAJ> 73
// 200424_160138     7.048 Rx FT4     -8  0.2 2798 CQ R9SDO LO91
// 200424_160138     7.048 Rx FT4     -9  0.0  292 CQ ON5ZZ JO11                       ? a1
// 200424_160145     7.048 Rx FT4      1 -0.0  504 R9SDO R2ASY +05

// "? a1" is about a priori decodes and their possible validity

// date-time rigfreq txrx mode s/n dt df message

//          bool is_new;

            trace(atline);
            QString id = "WSJTX";
            QTime time;
            qint32 snr;
            float delta_time;
            quint32 delta_frequency;
            QString mode;
            QString message;
            bool low_confidence = false;    // we sent it, after all

            QStringList sl = atline.trimmed().split(' ', QString::SkipEmptyParts);
            if (sl[2] != "Tx")
                continue;

            trace("Tx scraped: " + atline);
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


            decodeMessage dc = decoder.decode(id, eTX, time, snr, delta_time
                                            , delta_frequency, mode
                                            , message, low_confidence, true);

            trace(QString("WsjtxFrame::scrapeAllTxt - time %1 stage %2")
                  .arg(time.toString("HH:mm:ss")).arg(dc.getMStage()));

            currTxStage = dc.mstage;

            messages.push_back(dc);
            last = messages.end() - 1;

            decodes_model_->add_decode ();

            // process_decodes(); - not wanted, we've just started transmitting; previous process_decodes should have stopped us!
        }
    }
    return last;
}
void WsjtxFrame::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                , QString const& report, QString const& tx_mode, bool tx_enabled
                                , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, qint8 special_op_mode)
{
    MinosParameters *mp = MinosParameters::getMinosParameters();
    if (!mp)
        return;
    BaseContestLog * cc = mp ->getCurrentContest();
    if (ct != cc || cc == nullptr)
        return;
    id_ = id;

    decodeMessage *lastTx = nullptr;
    if (transmitting && !currentlyTransmitting)
    {
        // try scraping the transmissions from the all.txt file
        lastTx = scrapeAllTxt();
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
        case emsRRR:
            // repliable to "from"
            // G0XYZ K1ABC RR73
            // PA9XYZ G4ABC/P RR73
            workingCall = dx_call;
            break;
        }
        trace(QString("WsjtxFrame::update_status last tx stage %1 calling <%2> working <%3>").arg(lastTx->getMStage()).arg(callingCall).arg(workingCall));
    }


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
                              .arg (tx_mode.isEmpty () || tx_mode == mode ? QString() : '(' + tx_mode + ')'));


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
            QString mess = tr("<h1><b>Contest band %1 not the same as %2 band %3").arg(cb).arg(id).arg(bi.uk);
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
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (ct != cc)
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
            decode_added(true, "test", now, -1, 0, 0, "FT8", "G0GJV G3ZPB RR73", false, true);

            update_status ("test", 12345, "FT8", "","0", "FT8", false, false, false, 0, 0
                                    , "G0GJV", "IO91", "JO01"
                                    , false, "", false, 0);

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
