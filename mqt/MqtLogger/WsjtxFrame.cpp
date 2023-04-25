#include <QFileDialog>
#include <QListWidget>

#include "ContestApp.h"
#include "LoggerContest.h"
#include "AdifImport.h"
#include "MinosLoggerEvents.h"
#include "contacts.h"
#include "fileutils.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "htmldelegate.h"
#include "cutils.h"
#include "BandList.h"
#include "MTrace.h"

#include "WsjtxDecodesModel.hpp"
#include "WsjtxServer.h"
#include "WsjtxConfigureCQ.h"

#include "WsjtxFrame.h"
#include "ui_WsjtxFrame.h"

WsjtxFrame::WsjtxFrame(TSingleLogFrame *parent) :
    QFrame(parent)
  , tslf(parent)
  , ui(new Ui::WsjtxFrame)
  , decodes_model_ {new DecodesModel()}
{
    ui->setupUi(this);
    ui->decodes_table_view_->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->decodes_table_view_->horizontalHeader()->setSectionsMovable(true);
    connect( ui->decodes_table_view_->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &WsjtxFrame::ondecodes_table_view__customContextMenuRequested );

    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 1);
    remove_client(QString());    // kill off the ratshit
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX1TestEnabled, showTest );
    ui->testButton->setVisible(showTest);
    ui->replayButton->setVisible(showTest);

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpWSJTX1AutoEnabled, autoEnabled );
    ui->autoSelectReplyFrame->setVisible(autoEnabled);
    ui->blFrame->setVisible(autoEnabled);

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

    ui->decodes_table_view_->hideColumn (dcId);
//    ui->decodes_table_view_->hideColumn (dcDT);
//    ui->decodes_table_view_->hideColumn (dcDF);
    ui->decodes_table_view_->hideColumn (dcMd);
    ui->decodes_table_view_->hideColumn (dcConfidence);
    ui->decodes_table_view_->hideColumn (dcLive);
    ui->decodes_table_view_->hideColumn (dcSeq);
//    //ui->decodes_table_view_->hideColumn (dcPoints);
//    //ui->decodes_table_view_->hideColumn (dcBearing);
    ui->decodes_table_view_->hideColumn (dcDistance);
//    //ui->decodes_table_view_->hideColumn (dcFromCall);
//    //ui->decodes_table_view_->hideColumn (dcFromGrid);
//    //ui->decodes_table_view_->hideColumn (dcToCall);
//    ui->decodes_table_view_->hideColumn (dcToGrid);


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


    createColumnsMenu(columnsMenu, decodes_model_, this,
              [=]{
                    viewColumn();
              });

    connect( ui->decodes_table_view_->horizontalHeader(), &QHeaderView::sectionMoved,
             this, &WsjtxFrame::ondecodes_table_view__sectionMoved);
    connect( ui->decodes_table_view_->horizontalHeader(), &QHeaderView::sectionResized,
             this, &WsjtxFrame::on_sectionResized);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::doColumnChanges, this, &WsjtxFrame::on_doColumnChanges);

    restoreSplitters();
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::doSplitterChanges, this, &WsjtxFrame::on_doSplitterChanges);

    getCQStrings();

    blackList =    QSharedPointer<QVector <QSharedPointer<BlCall> > >( new QVector<QSharedPointer<BlCall> > );

    blModel.setCallVector(blackList);
    BLDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;
    blModel.delegate = BLDelegate;

    blFilterModel.setSourceModel(&blModel);
    ui->blackListView->setModel(&blFilterModel);


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
    trace("WsjtxFrame::on_halt_tx_button__clicked do_halt_tx");
    ui->replyto_label->setText("");
    WsjtxServer::getWsjtxServer()->do_halt_tx(id_, false);
}

void WsjtxFrame::on_auto_off_button__clicked()
{
    trace("WsjtxFrame::on_auto_off_button__clicked disable tx clicked");
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
    restoreWSJTXTableColumns();
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
        trace( "WsjtxFrame::log_ADIF Failed to append ADIF from " + id );
        return;
    }
    QSharedPointer<BaseContact> bct;
    for ( int i = spoint; i < ct->ctList.count(); i++ )
    {
        // do we ever get multiple QSOs in one ADIF? I don't think so...
        // not at least as sent from WSJT-X et al!
        bct = ct->pcontactAt(i);
        if (bct->loc.getLoc().isEmpty())
        {
            Callsign cs = bct->cs;
            const Locator loc = WsjtGetCallLoc(cs);
            bct->loc = loc;
            trace(QString("WsjtxFrame::log_ADIF loc for %1 is empty; filling in with %2").arg(cs.getFullCall(), loc.getLoc()));
        }
        bct->commonSave(bct);
    }
    ct->commonSave( false );
    ct->scanContest();      // after ADIF logged, could we do a single QSO scan? But this is only every few seconds
    for ( int i = spoint; i != ct->ctList.count(); i++ )
    {
        QSharedPointer<BaseContact> bct = ct->pcontactAt(i);
        MinosLoggerEvents::SendAfterLogContact(ct, bct); // after ADIF logged "last contact"
    }

    TSingleLogFrame * tslf = LogContainer ->findContest( ct );

    tslf->updateTrees();    // (log_ADIF complete redraw of QSO model...
    tslf->startNextEntry();       // log_ADIF
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
    ui->autoSelectButton->setChecked(false);
    id_.clear();
}
bool WsjtxFrame::goodCQCall(decodeMessage &dc)
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
            trace(QString("WsjtxFrame::goodCQCall %1 wrong CQ call").arg(dc.message));
            return false;
        }
    }
    else
    {
        if (!nonTestCQCalls.contains(dc.CQCall))
        {
            trace(QString("WsjtxFrame::goodCQCall %1 wrong CQ call").arg(dc.message));
            return false;
        }

    }
    return true;
}
void WsjtxFrame::getBestCQ73CallingMe()
{
    trace(QString("WsjtxFrame::getBestCQ73CallingMe"));

    for (int i = decodeStartSize; i < decodeEndSize; i++)
    {
        decodeMessage &dc = messages[i];
        if (dc.oldmsg)
            continue;
        if (dc.decodeInd[0] == '?')
            continue;

        trace(QString("WsjtxFrame::getBestCQ73CallingMe Checking %1 stage %2 tocall %3 fromcall %4")
                  .arg(messages[i].message)
                  .arg(dc.getMStage())
                  .arg(dc.toCall.getFullCall())
                  .arg(dc.fromCall.getFullCall()));

        if (dc.points <= 0)    // e.g. duplicate
            continue;

        PointBonusMultSnr pbv(dc);
        bool toMyCall = (dc.toCall == decoder.getMyCall());
        QString dcFromCall = dc.fromCall.getFullCall();

        BlCall test;
        test.call = dcFromCall;
        test.band = ct->currentBand.getValue();

        if (blackListContains(test))
        {
            trace("WsjtxFrame::getBestCQ73CallingMe blacklisted");
            continue;
        }

        if ((dc.mstage == emsCQ)   // CQ calls aren't "to" anyone
            || ((dc.mstage == emsGrid) && toMyCall)
            )
        {
            if (dc.mstage == emsCQ)
            {
                if (!goodCQCall(dc))
                {
                    // e.g. CQ DX in a contest
                    trace("WsjtxFrame::getBestCQ73CallingMe - not good CQ");
                    continue;
                }
            }
            if ( dc.snr >= minsnr
                && dc.points > minpoints
                && pbv > bestPoints
                )
            {
                trace(QString("WsjtxFrame::getBestCQ73CallingMe Candidate %1").arg(messages[i].message));
                bestOffset = i;
                bestPoints = pbv;
            }
            else
            {
                trace(QString("WsjtxFrame::getBestCQ73CallingMe NOT best %1").arg(messages[i].message));
            }
        }
        else
        {
            trace(QString("WsjtxFrame::getBestCQ73CallingMe NOT Candidate %1").arg(messages[i].message));
        }
    }
}
bool WsjtxFrame::getBestToMe()
{
    trace(QString("WsjtxFrame::getBestToMe"));

    for (int i = decodeStartSize; i < decodeEndSize; i++)
    {
        decodeMessage &dc = messages[i];
        if (dc.oldmsg)
            continue;
        if (dc.decodeInd[0] == '?')
            continue;   // potentially bad decode

        trace(QString("WsjtxFrame::getBestToMe Checking against lastTx %1 stage %2 tocall %3 fromcall %4 callingCall %5 workingCall %6")
                  .arg(dc.message).arg(dc.getMStage())
                  .arg(dc.toCall.getFullCall())
                  .arg(dc.fromCall.getFullCall())
                  .arg(callingCall)
                  .arg(workingCall)
              );

        // if we are calling CQ or RR73 or 73, and we are toCall, we have a set of candidates
        // for best work these before lookng for others

        //  WSJT-X filters this message and only
        // acts upon it  if the message exactly describes  a prior decode
        // and that decode  is a CQ or QRZ message.

        PointBonusMultSnr pbv(dc);
        bool toMyCall = (dc.toCall == decoder.getMyCall());
        QString dcFromCall = dc.fromCall.getFullCall();

        BlCall test;
        test.call = dcFromCall;
        test.band = ct->currentBand.getValue();

        if (blackListContains(test))
        {
            trace("WsjtxFrame::getBestToMe() blacklisted");
            continue;
        }

        if (toMyCall)
        {
            trace("WsjtxFrame::getBestToMe() toMyCall");
            if (currTxStage == emsCQ || currTxStage == emsRRR || currTxStage == ems73)
            // calling CQ or waiting for 73
            {
                // look for best candidate of those calling us - don't limit by snr or points
                // If they are starting with Tx2, we can miss the loc - and so their score will
                // be miniscule (probably 0)
                // so start bestpoints at -1...
                if ( pbv > bestPoints  )
                {
                    // which might not be the most profitable
                    trace(QString("WsjtxFrame::getBestToMe (lasttx) Candidate %1")
                              .arg(dc.message));
                    bestOffset = i;
                    bestPoints = pbv;
                                    }
                else
                {
                    trace(QString("WsjtxFrame::getBestToMe (lasttx) NOT best %1")
                              .arg(dc.message));
                }
            }
            else if (dcFromCall == workingCall || dcFromCall == callingCall)
            {
                trace("WsjtxFrame::getBestToMe (dcFromCall == workingCall || dcFromCall == callingCall)");
                // this is best, and WSJT-X should automatically respond
                // so long as autoseq is set in WSJT-X

                ui->decodes_table_view_->scrollToBottom ();
                return true;
            }
            else
            {
                trace("WsjtxFrame::getBestToMe ignore");
                // ignore it - random caller with us in mid QSO
                continue;
            }
        }
        else
        {
            trace("WsjtxFrame::getBestToMe Not toMyCall");
        }
    }
    return false;
}
bool WsjtxFrame::checkThemPresent()
{
    // check if the station we are trying to work is active
    // either calling someone else of being called by someone else
    // which would imply that they have swapped time periods

    for (int i = decodeStartSize; i < decodeEndSize; i++)
    {
        decodeMessage &dc = messages[i];
        if (dc.oldmsg)
            continue;
        if (dc.decodeInd[0] == '?')
            continue;   // potentially bad decode

        trace(QString("WsjtxFrame::checkThemPresent Checking against lastTx %1 stage %2 tocall %3 fromcall %4 callingCall %5 workingCall %6")
                  .arg(dc.message).arg(dc.getMStage())
                  .arg(dc.toCall.getFullCall())
                  .arg(dc.fromCall.getFullCall())
                  .arg(callingCall)
                  .arg(workingCall)
              );
        QString dcFromCall = dc.fromCall.getFullCall();
        if (dcFromCall == callingCall || dcFromCall == workingCall)
        {
            trace(QString("WsjtxFrame::checkThemPresent (dcFromCall == callingCall || dcFromCall == workingCall)"));
            return true;
        }
    }
    return false;
}
bool WsjtxFrame::checkTheirCall()
{
    trace(QString("WsjtxFrame::checkTheirCall"));
    for (int i = decodeStartSize; i < decodeEndSize; i++)
    {
        decodeMessage &dc = messages[i];
        if (dc.oldmsg)
            continue;
        if (dc.decodeInd[0] == '?')
            continue;   // potentially bad decode

        trace(QString("WsjtxFrame::checkTheirCall Checking against lastTx %1 stage %2 tocall %3 fromcall %4 callingCall %5 workingCall %6")
                  .arg(dc.message).arg(dc.getMStage())
                  .arg(dc.toCall.getFullCall())
                  .arg(dc.fromCall.getFullCall())
                  .arg(callingCall)
                  .arg(workingCall)
              );
        bool toMyCall = (dc.toCall == decoder.getMyCall());
        QString dcFromCall = dc.fromCall.getFullCall();
        bool cq = (dc.mstage == emsCQ);

        if (!toMyCall)
        {
            trace("WsjtxFrame::checkTheirCall !toMyCall");
            if (!cq && (dcFromCall == callingCall || dcFromCall == workingCall))
            {
                // we are trying to work them, and they aren't working us
                // - still CQ, or working someone else

                // If they are calling CQ and we are "grid" we can carry on calling them
                // don't kill tx unless there is a better option - using the general best search

                // bestOffset should already be -1 unless there is someone else calling us
                // in which case we will switch to them

                trace(QString("WsjtxFrame::checkTheirCall (lasttx) stop response, look again"));
                on_halt_tx_button__clicked();          // kill the automatic sequencing
                qsoState = NoQSOWaiting;
                trace("WsjtxFrame::checkTheirCall() Transition to NoQSOWaiting");

                return false;
            }
        }
        else
        {
            trace(QString("WsjtxFrame::checkThemPresent toMyCall, ignore"));

        }
    }
    trace(QString("WsjtxFrame::checkTheirCall returns true"));

    return true;
}

void WsjtxFrame::markBest()
{
    trace("WsjtxFrame::markBest");
    if (bestOffset >= 0)
    {
        for (int i = decodeStartSize; i < decodeEndSize; i++)
        {
            decodeMessage &dc = messages[i];
            dc.best = (i == bestOffset);
        }
        trace("WsjtxFrame::markBest best decode is " + messages[bestOffset].message);
    }
}
QString WsjtxFrame::getStateText(QSOStates s)
{
    QString t;
    switch(s)
    {
    case NoQSOWaiting:
        t = "NoQSOWaiting";
        break;
    case NoQSOCallingCQ:
        t = "NoQSOCallingCQ";
        break;
    case NoQSOCallingThem:
        t ="NoQSOCallingThem";
        break;
    case InQSO:
        t = "InQSO";
        break;
    }
    return t;
}
void WsjtxFrame::doResponse(QSOStates nstate, bool cq)
{
    trace("WsjtxFrame::doResponse");
    if (ui->autoSelectButton->isChecked() && !currentlyTransmitting)
    {
        if ( bestOffset >= 0)
        {
            trace("WsjtxFrame::doResponse auto replying to " + messages[bestOffset].message);
            messages[bestOffset].autoresp = true;
            reply(messages[bestOffset]);

            qsoState = nstate;
            trace("WsjtxFrame::doResponse Transition to " + getStateText(nstate));

            // we are assuming that autoseq is enabled, call 1st isn't
            // but we can't enforce either
            // things may be messy if we are not set this way
        }
        else if (cq)
        {
            // If we can, start calling CQ
            startCQ();
        }
    }
}

void WsjtxFrame::process_NoQSOWaiting()
{
    // Not calling CQ, not in QSO - look for the best CQ, or someone calling us
    // out of the blue

    // iterate over the latest decodes, and select the best

    // currTxStage will be emsNone

    trace("WsjtxFrame::process_NoQSOWaiting()");
    bestOffset = -1;
    bestPoints.clear();

    getBestCQ73CallingMe();

    markBest();

    doResponse(NoQSOCallingThem, true);
}
void WsjtxFrame::process_NoQSOCallingCQ()
{
    // Not in QSO, calling CQ
    // look for the best reply to our CQ, if none, look for the best CQ to call

    trace("WsjtxFrame::process_NoQSOCallingCQ()");
    bestOffset = -1;
    bestPoints.clear();

    QSOStates nState = NoQSOCallingCQ;
    // iterate over the latest decodes, and select the best

    // first, look at messages against our transmit status
    // NB we go to emsNone when (!transmitting && !tx_enabled)
    // which when rr73/73 has been sent, at end of
    // update_status()

    if (getBestToMe())
    {
        nState = InQSO;
    }
    else
    {
        // we don't already have a best
        getBestCQ73CallingMe();
        if (bestOffset >= 0)
        {
            nState = NoQSOCallingThem;
        }
    }

    markBest();
    doResponse(nState, false);
}
void WsjtxFrame::process_NoQSOCallingThem()
{
    // We have replied to a CQ, and are wanting them to come back to us
    // If they reply, let auto sequence proceed
    // if no reply, look for best calling us, or best CQ

    trace("WsjtxFrame::process_NoQSOCallingThem()");
    bestOffset = -1;
    bestPoints.clear();

    if (!checkTheirCall())
    {
        trace("WsjtxFrame::process_NoQSOCallingThem() !checkTheirCall()");
        // we have cancelled as they are CQ or working someone else
        // state is NoQSOWaiting
        // if they aren't in evidence, keep trying
        attempts = 0;
        getBestCQ73CallingMe();
        markBest();
        doResponse(NoQSOCallingThem, true);
    }
    else
    {
        if (checkThemPresent())
        {
            trace("WsjtxFrame::process_NoQSOCallingThem() checkThemPresent()");
            if (getBestToMe())
            {
                trace("WsjtxFrame::process_NoQSOCallingThem() getBestToMe()");
                attempts = 0;
                qsoState = InQSO;
                trace("WsjtxFrame::process_NoQSOCallingThem()Transition to InQSO");
            }
            else
            {
                // present, try 3 times and give up
                attempts++;
                trace(QString("WsjtxFrame::process_NoQSOCallingThem() !getBestToMe() attempts %1)").arg(attempts));

                if (attempts > 3)
                {
                    attempts = 0;

                    trace(QString("WsjtxFrame::process_NoQSOCallingThem() Add %1 to blacklist after 3 attempts").arg(dxCall));
                    on_addBlackListButton_clicked();
                    getBestCQ73CallingMe();
                    markBest();
                    doResponse(NoQSOCallingThem, false);
                }
            }
        }
        else
        {
            // not present, try 3 times and give up
            attempts++;
            trace(QString("WsjtxFrame::process_NoQSOCallingThem() !checkThemPresent() attempts %1").arg(attempts));
            if (attempts > 3)
            {
                attempts = 0;

                trace(QString("WsjtxFrame::process_NoQSOCallingThem() Add %1 to blacklist after 3 attempts").arg(dxCall));
                on_addBlackListButton_clicked();
                getBestCQ73CallingMe();

                markBest();

                doResponse(NoQSOCallingThem, true);

            }
        }
    }
}
void WsjtxFrame::process_InQSO()
{
    // In QSO - we are past the call/1st response
    // if they are working us, let autoseq proceed
    // Check for them working someone else
    // Check for too many repeats, may need to blacklist them

    trace("WsjtxFrame::process_InQSO()");
    bestOffset = -1;
    bestPoints.clear();

    if (!checkTheirCall())
    {
        trace("WsjtxFrame::process_InQSO() !checkTheirCall()");
        // we have cancelled as they are CQ or working someone else
        // state is NoQSOWaiting
        // if they aren't in evidence, keep trying
        attempts = 0;
        getBestCQ73CallingMe();
        markBest();
        doResponse(NoQSOCallingThem, true);
    }
    else
    {
        if (checkThemPresent())
        {
            trace("WsjtxFrame::process_InQSOm() checkThemPresent()");
            if (getBestToMe()) // hopefully they are now calling me
            {
                trace("WsjtxFrame::process_InQSO() getBestToMe()");
                // let autosequence do its job
                attempts = 0;
            }
            else
            {
                // present, try 3 times and give up
                attempts++;
                trace(QString("WsjtxFrame::process_InQSO() !getBestToMe() attempts %1").arg(attempts));

                if (attempts > 3)
                {
                    attempts = 0;

                    trace(QString("WsjtxFrame::process_InQSO() Add %1 to blacklist after 3 attempts").arg(dxCall));
                    on_addBlackListButton_clicked();
                    getBestCQ73CallingMe();
                    markBest();
                    doResponse(NoQSOCallingThem, false);
                }
            }
        }
        else
        {
            // not present, try 3 times and give up
            attempts++;
            trace(QString("WsjtxFrame::process_InQSO() !checkThemPresent() attempts %1").arg(attempts));
            if (attempts > 3)
            {
                attempts = 0;

                trace(QString("WsjtxFrame::process_InQSO() Add %1 to blacklist after 3 attempts").arg(dxCall));
                on_addBlackListButton_clicked();
                getBestCQ73CallingMe();

                markBest();

                doResponse(NoQSOCallingThem, true);

            }
        }
    }
}

void WsjtxFrame::process_decodes()
{
    if (!bandOK)
        return;

    if (ui->autoSelectButton->isChecked())
    {
        decodeEndSize = messages.size();
        minpoints = ui->minPointsSpinner->value();
        if (!ui->minPointsCheckBox->isChecked())
            minpoints = 0;
        minsnr =  ui->snrSpinner->value();
        if (!ui->snrCheckBox->isChecked())
            minsnr = -100;
        trace(QString("WsjtxFrame::process_decodes Checking decodes start %1 end %2 autoselect %3 minPoints %4 minSnr %5")
                  .arg(decodeStartSize)
                  .arg(decodeEndSize)
                  .arg(ui->autoSelectButton->isChecked())
                  .arg(minpoints)
                  .arg(minsnr));

        if (decodeEndSize > decodeStartSize)
        {
            switch (qsoState)
            {
            case NoQSOWaiting:
                process_NoQSOWaiting();
                break;
            case NoQSOCallingCQ:
                process_NoQSOCallingCQ();
                break;
            case NoQSOCallingThem:
                process_NoQSOCallingThem();
                break;
            case InQSO:
                process_InQSO();
                break;
            }
            emit decodes_model_->dataChanged(decodes_model_->index(decodeStartSize, dcBest), decodes_model_->index(decodeEndSize, dcBest));
        }
    }

    ui->decodes_table_view_->scrollToBottom ();
}
decodeMessage *WsjtxFrame::parse_tx_message(QString atline, bool fromScrape)
{
    trace("In WsjtxFrame::parse_tx_message");
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
          .arg(time.toString("HH:mm:ss"), dc.getMStage(), atline));

    currTxStage = dc.mstage;

    messages.push_back(dc);

    decodeMessage *last =  &messages.last();
    return last;
}

void WsjtxFrame::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
                                , QString const& report, QString const& tx_mode, bool tx_enabled
                                , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
                                , QString const& de_call, QString const& de_grid, QString const& dx_grid
                                , bool watchdog_timeout, QString const& sub_mode, bool fast_mode, quint8 so_mode
                                , quint32 /*frequency_tolerance*/, quint32 /*tr_period*/
                                , QString const& /*configuration_name*/, QString const& tx_message)
{
    // protected contests aren't interesting
    if (!ct || ct->isReadOnly())
    {
        ui->qsoStateLabel->clear();
        return;
    }

    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    double df = f;
    bandOK = blist.findBand(df, bi);
    if (bandOK)
    {
        QString cb = ct->contestBands.getValue().trimmed();
        if (ct->isHF())
        {
            cb = bi->uk;
            if (bi->getType() == HF_BANDTYPE)
            {
                ct->setCurrentBand(cb);

                MinosLoggerEvents::SendContestBandChanged(ct);
                ui->bandErrorLabel->clear();
            }
            else
            {
                QString mess = tr("<h1><b>Contest band %1 not the same as %2 band %3").arg(ct->currentBand.getValue(), id, bi->uk);
                ui->bandErrorLabel->setText(HtmlFontColour(Qt::red) + mess);
                bandOK = false;
            }
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
                QString mess = tr("<h1><b>Contest band %1 not the same as %2 band %3").arg(cb, id, bi->uk);
                ui->bandErrorLabel->setText(HtmlFontColour(Qt::red) + mess);
                bandOK = false;
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
        // don't continue
        ui->qsoStateLabel->setText(tr("Wrong band"));
        return;
    }

    trace(QString("WsjtxFrame::update_status dx_call %1 dx_grid %2 transmitting %3 decoding %4 tx_enabled %5 tx_message %6")
              .arg(dx_call).arg(dx_grid).arg(transmitting).arg(decoding).arg(tx_enabled).arg(tx_message));


    id_ = id;
    special_op_mode = so_mode;
    dxCall = dx_call;

    decodeMessage *lastTx = nullptr;
    if (transmitting && !currentlyTransmitting)
    {
        // transition from not transmitting to transmitting
        // so look at the message transmitted, decode it
        // and add it to the message list

        QDateTime ut = QDateTime::currentDateTimeUtc();

        decodeMessage dc = decoder.decode(id, eTX, ut.time(), 0, 0
                                        , 0, mode
                                        , tx_message, false, true);

        trace(QString("WsjtxFrame::update_status, decode TX message - stage %1, %2")
              .arg(dc.getMStage(), tx_message));

        currTxStage = dc.mstage;

        messages.push_back(dc);

        lastTx =  &messages.last();

        decodes_model_->add_decode ();
        ui->decodes_table_view_->scrollToBottom ();
        decodeStartSize++;
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
            // shouldn't happen... maybe from "TUNE"
            break;
        case emsCQ:
            // repliable to "from"
            // CQ K1ABC FN42
            // CQ DX K1ABC FN42
            // CQ TEST G4ABC/P IO91

            qsoState = NoQSOCallingCQ;
            trace("WsjtxFrame::update_status Transition to NoQSOCallingCQ");
            break;
        case ems73:
            // after this may still get far end RRR or RR73
            // repliable to "from"
            // K1ABC G0XYZ 73
            // G4ABC/P PA9XYZ 73
            qsoState = NoQSOWaiting;
            trace("WsjtxFrame::update_status ems73 Transition to NoQSOWaiting");
            break;
        case emsFree:
            // no calls involved
            break;

        case emsGrid:
            // grid is sl[2] is 4 fig loc
            // K1ABC G0XYZ IO91
            // G4ABC/P PA9XYZ JO22
            callingCall = dx_call;
            qsoState = NoQSOCallingThem;
            trace("WsjtxFrame::update_status Transition to NoQSOCallingThem");
            break;

        case emsDb:
            // db is aything else? or it may be a free text message
            // on HF we alwasy miss out grid
            //G0XYZ K1ABC –19
            //PA9XYZ 590003 IO91NP
            qsoState = NoQSOCallingThem;
            trace("WsjtxFrame::update_status Transition to NoQSOCallingThem");
            workingCall = dx_call;
            break;
        case emsRplusDb:
            // repliable to "from" - but may have to wait for 73
            // K1ABC G0XYZ R-22
            // G4ABC/P R 570007 JO22DB
        case emsRplusGrid:
            // similar to emsRplusDb
        case emsRplusDbGrid:
        case emsDbGrid:
            workingCall = dx_call;
            break;
        case emsRRR:
            // repliable to "from"
            // G0XYZ K1ABC RR73
            // PA9XYZ G4ABC/P RR73
            workingCall = dx_call;
            qsoState = NoQSOWaiting;
            trace("WsjtxFrame::update_status emsRRR Transition to NoQSOWaiting");
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
    case 5: special = "[WW DIGI]"; break;
    case 6: special = "[Fox]"; break;
    case 7: special = "[Hound]"; break;
    case 8: special = "[ARRL DIGI]"; break;
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
        // transition to decoding
        // remember start of decoded messages for this period
        inDecode = true;
        decodeStartSize = messages.size();
    }
    if (inDecode && decoding == false)
    {
        // transition to not decoding
        // so look at what has been decoded

        inDecode = false;

        process_decodes();
    }

    // Diagnostic display of current qso state

    ui->qsoStateLabel->setText(getStateText(qsoState));
}

void WsjtxFrame::decode_added (bool is_new, QString const& id, QTime time
                               , qint32 snr, float delta_time
                               , quint32 delta_frequency, QString const& mode
                               , QString const& message, bool low_confidence
                               , bool off_air)
{
    if (!bandOK)    // but we don't know the decode band
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
        trace(QString("WsjtxFrame::decode_added - %1 new message <%2> stage %3 points %4 snr %5").arg(time.toString("HH:mm:ss")).arg(message).arg(dc.getMStage()).arg(dc.points).arg(dc.snr));
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
void WsjtxFrame::startCQ()
{
    // temporary solution

    trace("WsjtxFrame::startCQ() Transition to NoQSOWaiting");
    qsoState = NoQSOWaiting;

    // And we need to send F1 to WSJT-X
}
void WsjtxFrame::reply(decodeMessage &dc)
{
    WsjtxServer::getWsjtxServer()->reply(dc.id, dc.time, dc.snr, dc.delta_time, dc.delta_frequency, dc.mode, dc.message, dc.low_confidence,  QApplication::keyboardModifiers () >> 24);
    ui->replyto_label->setText("WsjtxFrame::reply Replying to: " + dc.message);
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
        trace("WsjtxFrame autoselect off");
    }
    else
    {
        trace("WsjtxFrame autoselect on");
    }
}

void WsjtxFrame::on_testButton_clicked()
{
// A collection of messages that we have at som time wanted to test

//    void WsjtxServer::update_status (QString const& id, Frequency f, QString const& mode, QString const& dx_call
//                                      , QString const& report, QString const& tx_mode, bool tx_enabled
//                                      , bool transmitting, bool decoding, qint32 rx_df, qint32 tx_df
//                                      , QString const& de_call, QString const& de_grid, QString const& dx_grid
//                                      , bool watchdog_timeout, QString const& sub_mode, bool fast_mode
//                                     , quint8 special_op_mode, quint32 frequency_tolerance, quint32 tr_period
//                                     , QString const& configuration_name, QString const& tx_message)


            update_status ("test", Frequency(144174000), "FT8", "","0", "FT8", false, false, true, 0, 0
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

#ifdef RUBBISH
            QTime now = QTime::currentTime();

            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ M/ZL1DRI", false, true);

            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0XYZ K1ABC -19", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "<G4ABC> <PA9XYZ> R 580071 JO22DB", false, true);

            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0GJV M0GXZ IO92", false, true);
            decode_added(true, "test", now, -2, 0, 0, "FT8", "G0GJV G8KWX -19", false, true);
            decode_added(true, "test", now, -1, 0, 0, "FT8", "G0GJV G3ZPB IO91", false, true);
            decode_added(true, "test", now, -1, 0, 0, "FT8", "G0GJV G3ZPB RR73", false, true);
            decode_added(true, "test", now, -19, 0, 0, "FT8", "<GB1945PJ> SP9DEM JO90", false, true);
            decode_added(true, "test", now, -19, 0, 0, "FT8", "<...> RW3SK KO94", false, true);


//            // Normal
            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ K1ABC FN42            a1", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC G0XYZ IO91", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0XYZ K1ABC -19", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC G0XYZ R-22", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G0XYZ K1ABC RR73", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC G0XYZ 73", false, true);


//            // EU VHF Contest

            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ TEST G4ABC/P IO91", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G4ABC/P PA9XYZ JO22", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "PA9XYZ 590003 IO91NP", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G4ABC/P R 570007 JO22DB", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "PA9XYZ G4ABC/P RR73", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G4ABC/P PA9XYZ 73", false, true);

//            //And in 2.2 (both callsigns, hashed)
//            //(Either callsign (or both) may have /P appended.)

            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ TEST G4ABC IO91", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "G4ABC PA9XYZ JO22", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "<PA9XYZ> <G4ABC> 570123 IO91NP", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "<G4ABC> <PA9XYZ> R 580071 JO22DB", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "PA9XYZ G4ABC RR73", false, true);


//            // NA VHF Contest
//            //(Either callsign (or both) may have /R appended. You can use RR73 in place of RRR, and the final 73 is optional)

            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ TEST K1ABC FN42", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC W9XYZ EN37", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "W9XYZ K1ABC R FN42", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "K1ABC W9XYZ RR73", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "W9XYZ K1ABC 73", false, true);

//            // test for Ken

//            //One thing I did notice was the predicted scores for unworked stations.
//            //It said 17 for IO83, 22 for IO93, and 78 for IO92 - they seem radically wrong.

//            //In my log G3YDY (JO01) is shown as 243Km and G4RRA (IO80) is shown as 297Km.
//            //The robot works out ODX as G3YDY at 351Km

            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ G3YDY JO01", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ G1FFF IO83", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ G2FFF IO93", false, true);
            decode_added(true, "test", now, -14, 0, 0, "FT8", "CQ G3FFF IO92", false, true);

            update_status ("test", Frequency(144174000), "FT8", "","0", "FT8", false, false, false, 0, 0
                                    , "G0GJV", "IO91", "JO01"
                                    , false, "", false, 0, 0, 0, "", "");
#endif

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
void WsjtxFrame::on_configCQButton_clicked()
{
    WsjtxConfigureCQ wccq(this);

    wccq.exec();

    getCQStrings();
}

void WsjtxFrame::on_decodes_table_view__clicked(const QModelIndex &index)
{
    // How do we say "use from call"?

    Callsign call;
    decodeMessage &dc = messages[index.row()];

    if (index.column() == dcMessage)
    {
        switch (dc.mstage)
        {
        case emsNone:
            break;
        case emsFree:
            break;
        default:
            call = dc.fromCall;
            break;
        }
        if (call == decoder.getMyCall())
        {
            call = dc.toCall;
        }
    }
    // BUT we don't show these...
    else if (index.column() == dcFromCall)
    {
        call = dc.fromCall;
    }
    else if (index.column() == dcToCall)
    {
        call = dc.toCall;
    }
    if (!call.getFullCall().isEmpty())
    {
        if (call != decoder.getMyCall())
        {
            TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
            tslf->transferFromWSJTX(call.getFullCall());
        }
    }
}

void WsjtxFrame::doReplayTimer()
{
    if ( replayEnabled && fos.isOpen())
    {
        qint64 res = -1;
        if (!fos.atEnd())
        {
            qint16 instance;
            QDateTime tnow;
            QByteArray datagram;

            os >> instance;
            os >> tnow;
            os >> datagram;

            QUdpSocket bc;
            quint16 wsjtxRbPort = 2237;
            QHostAddress wsjtxRbHost(QHostAddress::LocalHost);
            res = bc.writeDatagram(datagram, wsjtxRbHost, wsjtxRbPort);

            if ( res < 0 )
            {
                replayEnabled = false;
                fos.close();
                ui->replayButton->setText("Replay");
                return;
            }
        }
        else
        {
            disconnect(replayTimer, &QTimer::timeout, this, &WsjtxFrame::doReplayTimer);
            replayTimer->deleteLater();
            replayTimer = nullptr;
            fos.close();
        }
    }
}
void WsjtxFrame::on_replayButton_clicked()
{
    if (!fos.isOpen())
    {
        QString dpath = WsjtxServer::getUdpRecPath();

        QString InitialDir = GetCurrentDir() + "/" + dpath;

        QString Filter = tr("WSJT-X recording Files") + " (*.wsjtx);;" +
                         tr("All Files") + " (*.*)" ;

        QString baseFileName = QFileDialog::getOpenFileName( this,
                           tr("WSJT-X recording Files"),
                           InitialDir,                   // opendir
                           Filter );

        if ( !baseFileName.isEmpty() )
        {
            fos.setFileName(baseFileName);
            if (!fos.open(QIODevice::ReadOnly))
               return;

            os.setDevice(&fos);
            os.setVersion (QDataStream::Qt_5_4);

            replayEnabled = true;
            ui->replayButton->setText("Pause Replay");
            replayTimer = new QTimer(this);
            connect(replayTimer, &QTimer::timeout, this, &WsjtxFrame::doReplayTimer);
            replayTimer->start(100);
        }
    }
    else
    {
        replayEnabled = !replayEnabled;
        if (replayEnabled)
        {
            ui->replayButton->setText("Pause Replay");
        }
        else
        {
            ui->replayButton->setText("Restart Replay");
        }
    }

}

void WsjtxFrame::viewColumn()
{
    // a columnsMenu entry has been clicked... action it
    QAction *act = dynamic_cast<QAction *>(sender());
    if (act)
    {
        int col = act->data().toInt();
        if (col >= 0)
        {
            bool check = act->isChecked();
            ui->decodes_table_view_->horizontalHeader()->setSectionHidden(col, !check);
        }
        else
        {
            TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
            QString fname("./Configuration/LoggerTableHeaders.ini");
            resetHeaderColumns(fname, "WSJTXDecodesTable", tslf->getCurScreenLayout(), ui->decodes_table_view_->horizontalHeader());
        }
    }
    saveWSJTXTableColumns();
}
void WsjtxFrame::saveWSJTXTableColumns()
{
    if (!inRestoreColumns && firstRestoreDone)
    {
        QString fname("./Configuration/LoggerTableHeaders.ini");
        saveHeaderColumns(fname, "WSJTXDecodesTable", tslf->getCurScreenLayout(), ui->decodes_table_view_->horizontalHeader());
        MinosLoggerEvents::SendColumnsChanged();
    }
}
void WsjtxFrame::restoreWSJTXTableColumns()
{
    inRestoreColumns = true;
    QString fname("./Configuration/LoggerTableHeaders.ini");
    restoreHeaderColumns(fname, "WSJTXDecodesTable", tslf->getCurScreenLayout(), ui->decodes_table_view_->horizontalHeader());
    inRestoreColumns = false;
    firstRestoreDone = true;
}
void WsjtxFrame::ondecodes_table_view__customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->decodes_table_view_->mapToGlobal( pos );
    popupColumnsMenu(columnsMenu, globalPos, ui->decodes_table_view_->horizontalHeader());
}
void WsjtxFrame::ondecodes_table_view__sectionMoved(int, int, int)
{
    saveWSJTXTableColumns();
}
void WsjtxFrame::on_sectionResized(int, int , int)
{
    saveWSJTXTableColumns();
}
void WsjtxFrame::on_doColumnChanges(BaseContestLog * b)
{
    if (b == ct)
    {
        restoreWSJTXTableColumns();
    }
}

void WsjtxFrame::on_addBlackListButton_clicked()
{
    // add current call being "worked" to blacklist

    QSharedPointer<BlCall> bc(new BlCall);
    bc->call = dxCall;
    if (!bc->call.isEmpty())
    {
        bc->band = ct->currentBand.getValue();
        blModel.appendRow(bc);
        on_halt_tx_button__clicked();

        showBlackList();
    }
}


void WsjtxFrame::on_removeBlackListButton_clicked()
{
    // remove selected call from blacklist
    QModelIndexList mil = ui->blackListView->selectionModel()->selectedRows();

    for(auto &mi: mil)
    {
        QModelIndex m = blFilterModel.mapToSource(mi);
        int r = m.row();

        if (r >= 0)
        {
            blModel.removeRow(r);
        }
    }
}

void WsjtxFrame::showBlackList()
{
    ui->blackListView->update();    // not right!
}

bool WsjtxFrame::blackListContains(const BlCall &bl)
{
    for(const auto &b:qAsConst(*blackList))
    {
        if (*b == bl)
        {
            return true;
        }
    }
    return false;
}
BlModel::BlModel(){}

BlModel::~BlModel()
{}

void BlModel::setCallVector(QSharedPointer<QVector<QSharedPointer<BlCall> > > &pcallVector)
{
    beginResetModel();
    callVector = pcallVector;
    endResetModel();
}

QVariant BlModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return callVector->at(index.row())->call;
    }
    return QVariant();
}

QVariant BlModel::headerData(int /*section*/, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return tr("Call");
    }
    return QVariant();
}

QModelIndex BlModel::parent(const QModelIndex &/*index*/) const
{
    return QModelIndex();
}

QModelIndex BlModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!callVector)
        return QModelIndex();

    if ( row < 0 || row >= rowCount() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column, nullptr );

}

int BlModel::rowCount(const QModelIndex &/*parent*/) const
{
    if (!callVector)
        return 0;
    return callVector->count();
}

int BlModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

void BlModel::appendRow(QSharedPointer<BlCall> call)
{
    beginInsertRows(QModelIndex(), rowCount() , rowCount());
    callVector->push_back(call);
    endInsertRows();
}

void BlModel::insertRow(int row, QSharedPointer<BlCall> call)
{
    beginInsertRows(QModelIndex(), row , row);
    callVector->insert(row, call);
    endInsertRows();
}

void BlModel::reset()
{
    beginResetModel();
    callVector->clear();
    endResetModel();
}

void BlModel::removeRow(int _row)
{
    beginRemoveRows(QModelIndex(), _row, _row);
    callVector->removeAt(_row);
    endRemoveRows();
}

void BlModel::setFilterString(QString f)
{
    filterString = f;
}


bool BlGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    BlModel *cgm = dynamic_cast<BlModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QSharedPointer<BlCall> call = cgm->callVector->at(sourceRow);
    if (filterString.isEmpty() || call->band == filterString)
    {
        return true;
    }
    return false;
}

void BlGridSortFilterModel::setFilterString(QString f)
{
    BlModel *cgm = dynamic_cast<BlModel *>(sourceModel());
    if (cgm)
        cgm->setFilterString(f);

    filterString = f;
    invalidateFilter();
}

void WsjtxFrame::on_resetButton_clicked()
{
    on_halt_tx_button__clicked();          // kill the automatic sequencing
    qsoState = NoQSOWaiting;
    trace("WsjtxFrame::on_resetButton_clicked() Transition to NoQSOWaiting");
}

