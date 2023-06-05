#include <QThread>
#include <QSettings>
#include "AppStartup.h"
#include "regsettings.h"
#include "RPCCommandConstants.h"
#include "cutils.h"
#include "MinosRPC.h"
#include "RSConfigure.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "RSMainWindow.h"
#include "rigcontrolcommonconstants.h"
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
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createCloseEvent();

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    bool trackRig = settings.getSettings().value("trackRig", false).toBool();
    ui->trackRig->setChecked(trackRig);
    bool trackSub= settings.getSettings().value("trackSub", false).toBool();
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
    sv.prepend("");

    trace(sv.join(';'));
    rsc.setServerList(sv, mainRig.server, subRig.server);

    n1mmLink.disconnect();
    bool wcon = wsjtxLink.isConnected();
    if (wcon)
    {
        trace("Disconnecting WSJT-X link");
        wsjtxLink.disconnect();
    }
    else
    {
        trace("WSJT-X link not connected");
    }

    if (rsc.exec() == QDialog::Accepted)
    {
        mainRig.configureServer(rsc.getMainServer());
        subRig.configureServer(rsc.getSubServer());

        QStringList cb = subRig.populateRig();
        ui->Rig2Combo->clear();
        ui->Rig2Combo->addItems(cb);
        ui->Rig2Combo->setCurrentText(subRig.selected.toString());
    }
    n1mmLink.initialise();
    if (wcon)
    {
        trace("Reconnecting WSJT-X link");
        wsjtxLink.initialise();
    }
}
RSMainWindow::~RSMainWindow()
{
    delete ui;
}

void RSMainWindow::closeEvent(QCloseEvent *event)
{
    // and tidy up all loose ends

    syncTimerTimer( );

    QWidget::closeEvent(event);
}
void RSMainWindow::moveEvent(QMoveEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void RSMainWindow::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void RSMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue("geometry", saveGeometry());
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
    }
    if (mainRig.check(wsjtxLink))
    {
       ui->Rig1Rig->setText(wsjtxLink.getRadioName());
       if (ui->trackRig->isChecked())
       {
           on_transfer12Button_clicked();
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
    //trace("Transfer 1 - 2");
    if (mainRig.rigFreq.isClear() || (mainRig.rigFreq == subRig.rigFreq && mainRig.rigMode == subRig.rigMode))
    {
        //trace("transfer12 - No change required");
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

    //trace("Transfer 2 - 1");
    if (subRig.rigFreq.isClear() || (mainRig.rigFreq == subRig.rigFreq && mainRig.rigMode == subRig.rigMode))
    {
        //trace("transfer21 - No change required");
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

void RSMainWindow::on_notify( AnalysePubSubNotify an, const QString /*from*/ )
{
    // PubSub notifications
//    trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

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
        }
    }
    else
    {
        ui->Rig2Combo->setCurrentText(QString());
        ui->QF2Label->clear();
    }
}

//---------------------------------------------------------------------------
void RSMainWindow::on_routerCall(bool /*err*/, QSharedPointer<MinosRPCObj> /*mro*/, const QString /*from*/ )
{
//    trace( "router callback from " + from + ( err ? ":Error" : ":Normal" ) );
//    trace("method is " + mro->getMethodName());
}
//---------------------------------------------------------------------------

void RSMainWindow::on_trackRig_clicked()
{
    mainRig.setChoices(ui->trackRig->isChecked(), ui->trackSub->isChecked(), ui->wsjtxCb->isChecked());
    subRig.setChoices(ui->trackSub->isChecked(), ui->trackRig->isChecked(), ui->wsjtxCb->isChecked());

    if (ui->trackRig->isChecked() && !mainRig.rigFreq.isClear())
    {
        // set rig2 to rig1
        subRig.controlFreq(mainRig.rigFreq, mainRig.rigMode);
    }
    RegSettings settings;
    settings.getSettings().setValue("trackRig", ui->trackRig->isChecked());
}

void RSMainWindow::on_trackSub_clicked()
{
    mainRig.setChoices(ui->trackRig->isChecked(), ui->trackSub->isChecked(), ui->wsjtxCb->isChecked());
    subRig.setChoices(ui->trackSub->isChecked(), ui->trackRig->isChecked(), ui->wsjtxCb->isChecked());

    if (ui->trackSub->isChecked() && !subRig.rigFreq.isClear())
    {
        // set rig1 to rig2
        mainRig.controlFreq(subRig.rigFreq, subRig.rigMode);
    }

    RegSettings settings;
    settings.getSettings().setValue("trackSub", ui->trackSub->isChecked());
}

void RSMainWindow::on_wsjtxCb_stateChanged(int /*arg1*/)
{
    mainRig.setChoices(ui->trackRig->isChecked(), ui->trackSub->isChecked(), ui->wsjtxCb->isChecked());
    subRig.setChoices(ui->trackSub->isChecked(), ui->trackRig->isChecked(),  ui->wsjtxCb->isChecked());

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
        // deselect whatever was there
        subRigSelection(sel, false);
    }

    subRigSelection(name, true);
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
bool SyncRadio::check(WsjtxLink &wsjtxLink)
{
    if (wsjtxLink.isConnected())
    {
        Frequency f = wsjtxLink.getFrequency();
        if (f.isOK() )
        {
            rigFreq = wsjtxLink.getFrequency();
            rigMode = wsjtxLink.getMode();
            return true;
        }
    }
    return false;
}
void SyncRadio::setChoices(bool trthis, bool trother,  bool tw)
{
    trackThis = trthis;
    trackOther = trother;
    trackWSJTX = tw;
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
    //trace(QString("Send %1 to %2").arg(lFreq.traceStr(), which));
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
   // trace(QString(which + " master set to %1").arg(makeStr(m)));
    master = m;
}
