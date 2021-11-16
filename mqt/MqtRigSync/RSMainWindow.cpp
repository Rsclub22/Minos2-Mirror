#include "base_pch.h"

#include <QThread>
#include "cutils.h"
#include "MinosRPC.h"
#include "ConfigFile.h"
#include "rigutils.h"
#include "BandList.h"
#include "delayedaction.h"
#include "RSConfigure.h"
#include "RSMainWindow.h"
#include "ui_RSMainWindow.h"

static const char *rigSyncUuid = "RigSync";

static const int masterHoldTime = 1000;

RSMainWindow::RSMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RSMainWindow),
    n1mmLink(parent),
    wsjtxLink(parent)
{
    ui->setupUi(this);

    connect(&stdinReader,&StdInReader::stdinLine, this, &RSMainWindow::onStdInRead);
    stdinReader.start();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createCloseEvent();

    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    bool trackBand = settings.value("trackBand", false).toBool();
    ui->trackBandcb->setChecked(trackBand);

    bool trackRig = settings.value("trackRig", false).toBool();
    ui->trackRig->setChecked(trackRig);
    bool trackSub= settings.value("trackSub", false).toBool();
    ui->trackSub->setChecked(trackSub);

    connect(&SyncTimer, &QTimer::timeout, this, &RSMainWindow::syncTimerTimer);
    SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName(), false);

    connect(rpc, &MinosRPC::routerCall, this, &RSMainWindow::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &RSMainWindow::on_notify);

    QStringList sv = {rpcConstants::rigControlCategory
                      ,rpcConstants::rigDetailsCategory
                      ,rpcConstants::rigStateCategory
                     };
    rpc->initialiseRouters(sv);

    n1mmLink.initialise();

    configAction = new QAction( tr("Configure..."), this );
    ui->menuBar->addAction( configAction );
    connect(configAction, &QAction::triggered, this, &RSMainWindow::configure);

    connect(&claimTimer, &QTimer::timeout, this, &RSMainWindow::claimTimerTimer);

    ui->Rig2Combo->addItem(subRig.selected.toString());
    ui->Rig2Combo->setCurrentText(subRig.selected.toString());
}

void RSMainWindow::configure()
{
    QStringList sv;
    RSConfigure rsc(this);

    for (const auto &r: qAsConst(rigCache.getRigList()))
    {
        QString s = r.getRouterApp();
        sv.append(s);
    }
    sv.removeDuplicates();

    rsc.setServerList(sv, mainRig.server, subRig.server);

    if (rsc.exec() == QDialog::Accepted)
    {
        mainRig.configureServer(rsc.getMainServer());
        subRig.configureServer(rsc.getSubServer());

        QStringList cb = subRig.populateRig();
        ui->Rig2Combo->clear();
        ui->Rig2Combo->addItems(cb);
        ui->Rig2Combo->setCurrentText(subRig.selected.toString());
    }
}
RSMainWindow::~RSMainWindow()
{
    delete ui;
}

void RSMainWindow::onStdInRead(QString cmd)
{
    trace(QString("RSMainWindow::onStdInRead %1").arg(cmd));
    executeStdIn(cmd);
}
void RSMainWindow::closeEvent(QCloseEvent *event)
{
    // and tidy up all loose ends

    syncTimerTimer( );

    QWidget::closeEvent(event);
}
void RSMainWindow::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void RSMainWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void RSMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
void RSMainWindow::syncTimerTimer(  )
{
    static bool closed = false;
    if ( !closed )
    {
        if ( checkCloseEvent() )
        {
            closed = true;
            close();
        }
    }

    if (mainRig.check(n1mmLink))
    {
       ui->Rig1Rig->setText(n1mmLink.getRadioName());
       if (ui->trackRig->isChecked())
       {
           on_transfer12Button_clicked();
       }
       if (ui->trackBandcb->isChecked())
       {
            subRig.trackOtherBand(mainRig);
       }
    }
    if (mainRig.rigFreq.isClear())
    {
        ui->QF1Label->clear();
    }
    else
    {
        ui->QF1Label->setText(mainRig.rigFreq.convertFreqStrDisp());
    }
    if (subRig.rigFreq.isClear())
    {
        ui->QF2Label->clear();
    }
    else
    {
        ui->QF2Label->setText(subRig.rigFreq.convertFreqStrDisp());
    }

}

void RSMainWindow::claimTimerTimer()
{
    SyncRadio *lastMaster = nullptr;
    if (mainRig.isMaster())
    {
        lastMaster = &mainRig;
    }
    else if (subRig.isMaster())
    {
        lastMaster = &subRig;
    }
    trace("Master cleared");
    claimTimer.stop();
    mainRig.setMaster(false);
    subRig.setMaster(false);

    // now pick up any changes made by the sync radio since master's last tuning
    if (lastMaster == &mainRig && ui->trackSub->isChecked())
    {
        on_transfer21Button_clicked();
    }
    else if (lastMaster == &subRig && ui->trackRig->isChecked())
    {
        on_transfer12Button_clicked();
    }

}

void RSMainWindow::on_transfer12Button_clicked()
{
    // set sub rig to main rig
    trace("Transfer 1 - 2");
    if (mainRig.rigFreq.isClear() || (mainRig.rigFreq == subRig.rigFreq && mainRig.rigMode == subRig.rigMode))
    {
        trace("transfer12 - No change required");
        return;
    }
    mainRig.setMaster(true);
    subRig.setMaster(false);
    claimTimer.start(masterHoldTime);
    subRig.controlFreq(mainRig.rigFreq, mainRig.rigMode);
}


void RSMainWindow::on_transfer21Button_clicked()
{
    // set main rig to sub rig

    trace("Transfer 2 - 1");
    if (subRig.rigFreq.isClear() || (mainRig.rigFreq == subRig.rigFreq && mainRig.rigMode == subRig.rigMode))
    {
        trace("transfer21 - No change required");
        return;
    }
    mainRig.setMaster(false);
    subRig.setMaster(true);
    claimTimer.start(masterHoldTime);
    if (n1mmLink.isConnected())
    {
        n1mmLink.sendFrequencyRequest(subRig.rigFreq, subRig.rigMode);
    }
    else
    {
        mainRig.controlFreq(subRig.rigFreq, subRig.rigMode);
    }
}

void RSMainWindow::on_notify( AnalysePubSubNotify an, const QString from )
{
    // PubSub notifications
    trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

    if ( an.getOK() )
    {
        if ( an.getState() == psPublished)
        {
            if ( an.getCategory() == rpcConstants::rigStateCategory)
            {
                rigCache.setStateString(an);
            }
            else if ( an.getCategory() == rpcConstants::rigDetailsCategory)
            {
                rigCache.setDetailsString(an);
            }
            else if ( an.getCategory() == rpcConstants::rigControlCategory && an.getKey() == rpcConstants::rigControlRadioList )
            {
                rigCache.addRigList(an.getValue());

                ui->Rig1Rig->clear();
                QString pub = an.getPublisherRouter() + "/" + an.getPublisherProgram();

                if (pub == subRig.server)
                {
                    QStringList cb = subRig.populateRig();
                    ui->Rig2Combo->clear();
                    ui->Rig2Combo->addItems(cb);
                    ui->Rig2Combo->setCurrentText(subRig.selected.toString());

                    subRig.selectRadio(subRig.selected);
                }
            }
            else
                return;
        }

        for (const auto &r: qAsConst(rigCache.getRigList()))
        {
            if (r.getRouterApp() == mainRig.server)
            {
                QStringList selLogs = rigCache.getSelectedLoggers(r);
                if (r.appName() != rigSyncUuid && selLogs.count() && !selLogs.contains(rigSyncUuid))
                {
                    mainRig.selected = r;
                    break;
                }
            }
        }
        ui->Rig1Rig->setText(mainRig.selected.toString());

        if (!mainRig.selected.isEmpty())
        {
            RigState &selState = rigCache.getState(mainRig.selected);
            RigDetails &selDetail = rigCache.getDetails(mainRig.selected);
            ui->Rig1Rig->setText(mainRig.selected.toString());

            if (selDetail.isDirty())
            {
                selDetail.clearDirty();

            }
            if (selState.radioMode().isDirty() || selState.radioFreq().isDirty())
            {
                mainRig.rigMode = selState.radioMode().getValue().remove(":");
                mainRig.rigFreq = selState.radioFreq().getValue();
                trace(QString("main frequency changed to %1").arg(mainRig.rigFreq.traceStr()));

                selState.clearDirty();
                if (mainRig.rigFreq.isClear())
                {
                    ui->QF1Label->clear();
                }
                else
                {
                    ui->QF1Label->setText(subRig.rigFreq.convertFreqStrDisp());
                }

                if ((ui->trackRig->isChecked() && !subRig.isMaster()) || firstTime)
                {
                    trace(QString("firstTime 1 is %1").arg(makeStr(firstTime)));

                    if (!mainRig.rigFreq.isClear())
                    {
                        // first time - transfer rig to SDR, which will set up transverter
                        // settings as required

                        mainRig.setMaster(true);
                        subRig.setMaster(false);
                        claimTimer.start(masterHoldTime);

                        subRig.controlFreq(mainRig.rigFreq, mainRig.rigMode);
                        firstTime = false;
                    }
                    trace(QString("firstTime 2 is %1").arg(makeStr(firstTime)));
                }
                delayedAction(this, [=]{
                    if (ui->trackBandcb->isChecked() && mainRig.isMaster())
                    {
                         subRig.trackOtherBand(mainRig);
                    }
                }, 50);
            }
        }
        else
        {
            ui->Rig1Rig->clear();
            ui->QF1Label->clear();
        }
    }
    if (!subRig.selected.isEmpty())
    {
        RigState &selState = rigCache.getState(subRig.selected);
        RigDetails &selDetail = rigCache.getDetails(subRig.selected);

        if (selDetail.isDirty())
        {
            selDetail.clearDirty();

        }
        if (selState.radioMode().isDirty() || selState.radioFreq().isDirty())
        {
            subRig.rigMode = selState.radioMode().getValue().remove(":");
            subRig.rigFreq = selState.radioFreq().getValue();
            trace(QString("sync frequency changed to %1").arg(subRig.rigFreq.traceStr()));

            selState.clearDirty();
            if (subRig.rigFreq.isClear())
            {
                ui->QF2Label->clear();
            }
            else
            {
                ui->QF2Label->setText(subRig.rigFreq.convertFreqStrDisp());
            }


            if ((ui->trackSub->isChecked() && !mainRig.isMaster()) && !subRig.rigFreq.isClear() && !firstTime)
            {
                trace(QString("firstTime 3 is %1").arg(makeStr(firstTime)));
                mainRig.setMaster(false);
                subRig.setMaster(true);
                claimTimer.start(masterHoldTime);
                if (n1mmLink.isConnected())
                {
                    n1mmLink.sendFrequencyRequest(subRig.rigFreq, subRig.rigMode);
                }
                else
                {
                    mainRig.controlFreq(subRig.rigFreq, subRig.rigMode);
                }
            }
            delayedAction(this, [=]{
                if (ui->trackBandcb->isChecked() && mainRig.isMaster())
                {
                     subRig.trackOtherBand(mainRig);
                }
            }, 50);
        }
    }
    else
    {
        ui->Rig2Combo->setCurrentText(QString());
        ui->QF2Label->clear();
    }
}

//---------------------------------------------------------------------------
void RSMainWindow::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace( "router callback from " + from + ( err ? ":Error" : ":Normal" ) );
    trace("method is " + mro->getMethodName());
}
//---------------------------------------------------------------------------

void RSMainWindow::on_trackRig_clicked()
{
    mainRig.setChoices(ui->trackRig->isChecked(), ui->trackSub->isChecked(), ui->trackBandcb->isChecked(), ui->wsjtxCb->isChecked());
    subRig.setChoices(ui->trackSub->isChecked(), ui->trackRig->isChecked(), ui->trackBandcb->isChecked(), ui->wsjtxCb->isChecked());

    if (ui->trackRig->isChecked() && !mainRig.rigFreq.isClear())
    {
        // set rig2 to rig1
        subRig.controlFreq(mainRig.rigFreq, mainRig.rigMode);
    }
    QSettings settings;
    settings.setValue("trackRig", ui->trackRig->isChecked());
}

void RSMainWindow::on_trackSub_clicked()
{
    mainRig.setChoices(ui->trackRig->isChecked(), ui->trackSub->isChecked(), ui->trackBandcb->isChecked(), ui->wsjtxCb->isChecked());
    subRig.setChoices(ui->trackSub->isChecked(), ui->trackRig->isChecked(), ui->trackBandcb->isChecked(), ui->wsjtxCb->isChecked());

    if (ui->trackSub->isChecked() && !subRig.rigFreq.isClear())
    {
        // set rig1 to rig2
        mainRig.controlFreq(subRig.rigFreq, subRig.rigMode);
    }

    QSettings settings;
    settings.setValue("trackSub", ui->trackSub->isChecked());
}

void RSMainWindow::on_trackBandcb_stateChanged(int /*arg1*/)
{
    mainRig.setChoices(ui->trackRig->isChecked(), ui->trackSub->isChecked(), ui->trackBandcb->isChecked(), ui->wsjtxCb->isChecked());
    subRig.setChoices(ui->trackSub->isChecked(), ui->trackRig->isChecked(), ui->trackBandcb->isChecked(), ui->wsjtxCb->isChecked());

    QSettings settings;
    settings.setValue("trackBand", ui->trackBandcb->isChecked());

    mainRig.rigMode.clear();
    mainRig.lastRigMode.clear();

    subRig.rigFreq.clear();
    subRig.lastRigFreq.clear();

    mainRig.lastBand.clear();
    mainRig.lastBandMode.clear();

}

void RSMainWindow::on_wsjtxCb_stateChanged(int /*arg1*/)
{
    mainRig.setChoices(ui->trackRig->isChecked(), ui->trackSub->isChecked(), ui->trackBandcb->isChecked(), ui->wsjtxCb->isChecked());
    subRig.setChoices(ui->trackSub->isChecked(), ui->trackRig->isChecked(), ui->trackBandcb->isChecked(), ui->wsjtxCb->isChecked());

    if (ui->wsjtxCb->isChecked())
    {
        wsjtxLink.initialise();
    }
    else
    {
        wsjtxLink.disconnect();
    }
}

void RSMainWindow::on_Rig2Combo_activated(const QString &psn)
{
    subRig.selected = psn;

    QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RIGSYNC_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);

    config.setValue("SyncRigControlRig", subRig.selected.toString());

    subRig.selectRadio(subRig.selected);

}
//=========================================================================================

SyncRadio::SyncRadio(const QString &w, RigCache &r):which(w), rigCache(r)
{
    QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RIGSYNC_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);

    server = config.value(which + "RigControlApp", server).toString();
    selected = config.value(which + "RigControlRig", selected.toString()).toString();
}
void SyncRadio::configureServer(const QString s)
{
    PubSubName test = PubSubName(s + "/x");    // expects r/a/k - this is just r/a
    server = test.getRouterApp();

    QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RIGSYNC_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);

    config.setValue(which + "RigControlApp", server);
}
QStringList SyncRadio::populateRig()
{
    QStringList cb;
    cb.append("");
    for (const auto &r: qAsConst(rigCache.getRigList()))
    {
        if (!r.isEmpty() && r.getRouterApp() == server )
        {
            cb.append( r.toString());
        }
    }
    cb.removeDuplicates();
    cb.sort();
    return cb;
}

void SyncRadio::selectRadio(PubSubName name)
{
    PubSubName sel = rigCache.getSelected(rigSyncUuid);

    if (!sel.isEmpty() && sel != name)
    {
        subRigSelection(sel, false);
    }

    subRigSelection(sel, true);
}
bool SyncRadio::check(N1MMLink &n1mmLink)
{
    if (n1mmLink.isConnected())
    {
        Frequency f = n1mmLink.getFrequency();
        if (f.isOK() )
        {
            rigFreq = n1mmLink.getFrequency();
            rigMode = n1mmLink.getMode();
            return true;
        }
    }
    return false;
}
void SyncRadio::setChoices(bool trthis, bool trother, bool tb, bool tw)
{
    trackThis = trthis;
    trackOther = trother;
    trackBand = tb;
    trackWSJTX = tw;
}

void SyncRadio::trackOtherBand(SyncRadio &tracked)
{
    if (tracked.rigFreq == tracked.lastRigFreq && tracked.rigMode == tracked.lastRigMode)
    {
        return; // nothing to do, tracked rig hasn't changed
    }
    tracked.lastRigFreq = tracked.rigFreq;
    tracked.lastRigMode = tracked.rigMode;

    trace(QString("RSMainWindow::trackBand(): rig %1 mode %2").arg(tracked.lastRigFreq.traceStr(), tracked.lastRigMode));

    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOK = blist.findBand(tracked.rigFreq, bi);
    if (!bandOK)
    {
        return;
    }

    int modePart = -1;
    QSharedPointer<ModeInfo> mi = bi->findMode(tracked.rigMode, tracked.rigFreq, modePart);
    if (mi == lastBandMode && modePart ==lastModePart && bi == lastBand)
    {
        trace("band/mode unchanged");
        return;
    }
    if (!mi)
    {
        trace("band/mode not found");
        rigCentre(bi->fLow, bi->fHigh, tracked.rigMode);
    }
    else
    {
        trace("mode found OK");
        if (modePart == 1)
        {
            rigCentre(mi->fcLow1, mi->fcHigh1, tracked.rigMode);
        }
        else if (modePart == 2)
        {
            rigCentre(mi->fcLow2, mi->fcHigh2, tracked.rigMode);
        }
        else
        {
            rigCentre(mi->fLow, mi->fHigh, tracked.rigMode);
        }
    }
    lastBand = bi;
    lastBandMode = mi;
    lastModePart = modePart;
}

void SyncRadio::rigCentre(const Frequency &fLow, const Frequency &fHigh, const QString &mode)
{
    trace(QString("%1 %2 %3").arg(which, fLow.traceStr(), fHigh.traceStr()));
    qint64 bandWidth = qint64(fHigh) - qint64(fLow);
    qint64 centre = qint64(fLow) + bandWidth/2;
    centre += 12500;
    centre /= 25000;
    centre *= 25000;


    //qint64 lFreq = qint64(subRig.rigFreq);
    qint64 fCentre = centre;

    // tune rig2 to "fcentre"

    controlFreq(fCentre, mode);

}
void SyncRadio::subRigSelection(const PubSubName &sd, bool state)
{
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QString id = (state?rigSyncUuid:QString());

    rigCache.setSelected(sd, rigSyncUuid, id);

    st->addMember( rigSyncUuid, rpcConstants::loggerUuid );
    st->addMember( id, rpcConstants::selected );

    st->addMember( sd.toString(), rpcConstants::rigControlSelectRadioName );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( sd );

}
void SyncRadio::controlFreq(const Frequency &lFreq, QString mode)
{
    trace(QString("Send %1 to %2").arg(lFreq.traceStr(), which));
    QStringList qsl = rigCache.getSelectedLoggers(selected);
    if (qsl.count())
    {
        rigCache.setLogFreq(selected, lFreq);
        QString loggerUuid = qsl[0];
        QString selc = rigCache.getSelectedContest(selected, loggerUuid);

        {
            RPCGeneralClient rpc(rpcConstants::rigControlMethod);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember( loggerUuid, rpcConstants::loggerUuid );
            st->addMember( selc, rpcConstants::selected );
            st->addMember( lFreq.str(), rpcConstants::rigControlLogFreq );
            rpc.getCallArgs() ->addParam( st );
            rpc.queueCall( selected);
        }
        {
            RPCGeneralClient rpc(rpcConstants::rigControlMethod);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember( loggerUuid, rpcConstants::loggerUuid );
            st->addMember( selc, rpcConstants::selected );
            st->addMember( mode, rpcConstants::rigControlLogMode );
            rpc.getCallArgs() ->addParam( st );
            rpc.queueCall( selected);
        }
    }
}
bool SyncRadio::isMaster() const
{
    return master;
}
void SyncRadio::setMaster(bool m)
{
    trace(QString(which + " master set to %1").arg(makeStr(m)));
    master = m;
}
