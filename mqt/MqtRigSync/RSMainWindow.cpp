#include "base_pch.h"

#include <QThread>
#include "MinosRPC.h"
#include "ConfigFile.h"
#include "rigutils.h"
#include "BandList.h"
#include "delayedaction.h"
#include "RSConfigure.h"
#include "RSMainWindow.h"
#include "ui_RSMainWindow.h"

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

    connect(&SyncTimer, &QTimer::timeout, this, &RSMainWindow::SyncTimerTimer);
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
    ui->menuConfigure->addAction( configAction );
    connect(configAction, &QAction::triggered, this, &RSMainWindow::configure);

    QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RIGSYNC_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);

    subServer = config.value("SyncRigControlApp", subServer).toString();
    subRigSelected = config.value("SyncRigControlRig", subRigSelected.toString()).toString();
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

    rsc.setServerList(sv, subRigSelected.getRouterApp());

    if (rsc.exec() == QDialog::Accepted)
    {
        subRigSelected = PubSubName(rsc.getSubServer() + "/x");    // expects r/a/k - this is just r/a
        subServer = subRigSelected.getRouterApp();

        QStringList cb = populateRig2();
        ui->Rig2Combo->clear();
        ui->Rig2Combo->addItems(cb);
        ui->Rig2Combo->setCurrentText(subRigSelected.toString());

        QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RIGSYNC_CONFIG_FILE;
        QSettings config(fileName, QSettings::IniFormat);

        config.setValue("SyncRigControlApp", subServer);

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

    SyncTimerTimer( );

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
void RSMainWindow::SyncTimerTimer(  )
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

    if (n1mmLink.isConnected())
    {
        Frequency f = n1mmLink.getFrequency();
        if (f.isOK() )
        {
            mainRigFreq = n1mmLink.getFrequency();
            mainRigMode = n1mmLink.getMode();
            ui->Rig1Combo->setCurrentText(n1mmLink.getRadioName());
            if (ui->trackRig->isChecked())
            {
                on_transfer12Button_clicked();
            }
            trackBand();
        }
    }
    ui->QF1Label->setText(mainRigFreq.convertFreqStrDisp());
}

void RSMainWindow::on_closeButton_clicked()
{
    close();
}

void RSMainWindow::on_transfer12Button_clicked()
{
    // set sub rig to main rig
    subRigControlFreq(mainRigFreq, mainRigMode);
}


void RSMainWindow::on_transfer21Button_clicked()
{
    // set main rig to sub rig

    if (n1mmLink.isConnected())
    {
        n1mmLink.sendFrequencyRequest(subRigFreq, subRigMode);
    }
    else
    {
        mainRigControlFreq(subRigFreq, subRigMode);
    }
}
QStringList RSMainWindow::rigs()
{
    QStringList sl;
    QVector<PubSubName> riglist = rigCache.getRigList();
    for (const auto &psn: qAsConst(riglist))
    {
        QString rigname = psn.toString();
        sl.append(rigname);
    }
    sl.sort();
    return  sl;
}


QStringList RSMainWindow::populateRig2()
{
    QStringList cb;
    cb.append("");
    for (const auto &r: qAsConst(rigCache.getRigList()))
    {
        if (r.getRouterApp() == subRigSelected.getRouterApp() )
        {
            if (!r.isEmpty())
            cb.append( r.toString());
        }
    }
    cb.removeDuplicates();
    cb.sort();
    return cb;
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

                ui->Rig1Combo->clear();
                ui->Rig1Combo->addItem("");
                ui->Rig1Combo->addItems( rigs());

                QString pub = an.getPublisherRouter() + "/" + an.getPublisherProgram();

                if (pub == subRigSelected.getRouterApp())
                {
                    QStringList cb = populateRig2();
                    ui->Rig2Combo->clear();
                    ui->Rig2Combo->addItems(cb);
                    ui->Rig2Combo->setCurrentText(subRigSelected.toString());

                    selectRadio(subRigSelected);
                }
            }
            else
                return;
        }
        mainRigSelected = rigCache.getSelected("");
        ui->Rig1Combo->setCurrentText(mainRigSelected.toString());

        if (!mainRigSelected.isEmpty())
        {
            RigState &selState = rigCache.getState(mainRigSelected);
            RigDetails &selDetail = rigCache.getDetails(mainRigSelected);
            ui->Rig1Combo->setCurrentText(mainRigSelected.toString());

            if (selDetail.isDirty())
            {
                selDetail.clearDirty();

            }
            if (selState.isDirty())
            {
                mainRigMode = selState.radioMode().getValue().remove(":");
                mainRigFreq = selState.radioFreq().getValue();

                selState.clearDirty();
                ui->QF1Label->setText(mainRigFreq.convertFreqStrDisp());

                if (ui->trackRig->isChecked())
                {
                    on_transfer12Button_clicked();
                }
                delayedAction(this, [=]{
                    trackBand();
                }, 50);
            }
        }
        else
        {
            ui->Rig1Combo->setCurrentText("");
            ui->QF1Label->setText("");
        }
    }
    if (!subRigSelected.isEmpty())
    {
        RigState &selState = rigCache.getState(subRigSelected);
        RigDetails &selDetail = rigCache.getDetails(subRigSelected);
//        ui->Rig2Combo->setCurrentText(subRigSelected.toString());

        if (selDetail.isDirty())
        {
            selDetail.clearDirty();

        }
        if (selState.isDirty())
        {
            subRigMode = selState.radioMode().getValue().remove(":");
            subRigFreq = selState.radioFreq().getValue();

            selState.clearDirty();
            ui->QF2Label->setText(subRigFreq.convertFreqStrDisp());

            if (ui->trackSub->isChecked())
            {
                on_transfer21Button_clicked();
            }
            delayedAction(this, [=]{
                trackBand();
            }, 50);
        }
    }
    else
    {
        ui->Rig2Combo->setCurrentText("");
        ui->QF2Label->setText("");
    }
}

//---------------------------------------------------------------------------
void RSMainWindow::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace( "router callback from " + from + ( err ? ":Error" : ":Normal" ) );
    trace("method is " + mro->getMethodName());
}
//---------------------------------------------------------------------------
void RSMainWindow::rig2Centre(const Frequency &fLow, const Frequency &fHigh)
{
    trace(QString("%1 %2").arg(fLow.traceStr(), fHigh.traceStr()));
    qint64 bandWidth = qint64(fHigh) - qint64(fLow);
    qint64 centre = qint64(fLow) + bandWidth/2;
    centre += 12500;
    centre /= 25000;
    centre *= 25000;


    //qint64 lFreq = qint64(mainRigFreq);
    qint64 fCentre = centre;

    // tune rig2 to "fcentre"

    subRigControlFreq(fCentre, mainRigMode);

}
void RSMainWindow::trackBand()
{
    if (mainRigFreq == lastMainRigFreq && mainRigMode == lastMainRigMode)
    {
        return; // nothing to do
    }
    lastMainRigFreq = mainRigFreq;
    lastMainRigMode = mainRigMode;

    trace(QString("RSMainWindow::trackBand(): rig %1 mode %2").arg(lastMainRigFreq.traceStr(), lastMainRigMode));

    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOK = blist.findBand(mainRigFreq, bi);
    if (!bandOK)
    {
        return;
    }

    if (!ui->trackBandcb->isChecked())
    {
        return;
    }
    int modePart = -1;
    QSharedPointer<ModeInfo> mi = bi->findMode(mainRigMode, mainRigFreq, modePart);
    if (mi == lastBandMode && modePart == lastModePart && bi == lastBand)
    {
        trace("band/mode unchanged");
        return;
    }
    if (!mi)
    {
        trace("band/mode not found");
        rig2Centre(bi->fLow, bi->fHigh);
    }
    else
    {
        trace("mode found OK");
        if (modePart == 1)
        {
            rig2Centre(mi->fcLow1, mi->fcHigh1);
        }
        else if (modePart == 2)
        {
            rig2Centre(mi->fcLow2, mi->fcHigh2);
        }
        else
        {
            rig2Centre(mi->fLow, mi->fHigh);
        }
    }
    lastBand = bi;
    lastBandMode = mi;
    lastModePart = modePart;
}

void RSMainWindow::on_noTrack_clicked()
{
    // do nothing
}

void RSMainWindow::on_trackRig_clicked()
{
    // set rig2 to rig1
    on_transfer12Button_clicked();
}

void RSMainWindow::on_trackSub_clicked()
{
    // set rig1 to rig2
    on_transfer21Button_clicked();
}

void RSMainWindow::on_trackBandcb_stateChanged(int /*arg1*/)
{
    QSettings settings;
    settings.setValue("trackBand", ui->trackBandcb->isChecked());

    mainRigMode.clear();
    lastMainRigMode.clear();

    mainRigFreq.clear();
    lastMainRigFreq.clear();

    lastBand.clear();
    lastBandMode.clear();

}

void RSMainWindow::on_wsjtxCb_stateChanged(int /*arg1*/)
{
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
    subRigSelected = psn;

    QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RIGSYNC_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);

    config.setValue("SyncRigControlRig", subRigSelected.toString());

    selectRadio(subRigSelected);

}
void RSMainWindow::mainRigControlFreq(const Frequency &lFreq, QString mode)
{
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QStringList qsl = rigCache.getSelectedLoggers(mainRigSelected);
    if (qsl.count())
    {
        QString loggerUuid = qsl[0];

        st->addMember( loggerUuid, rpcConstants::loggerUuid );

        QString selc = rigCache.getSelectedContest(mainRigSelected, loggerUuid);

        st->addMember( selc, rpcConstants::selected );
        st->addMember( lFreq.str(), rpcConstants::rigControlLogFreq );
        st->addMember( mode, rpcConstants::rigControlLogMode );

        rpc.getCallArgs() ->addParam( st );

        rpc.queueCall( mainRigSelected);
    }
}

void RSMainWindow::selectRadio(PubSubName name)
{
    PubSubName selected = rigCache.getSelected(rigSyncUuid);

    if (!selected.isEmpty() && selected != name)
    {
        subRigSelection(selected, false);
    }

    subRigSelection(subRigSelected, true);
}
void RSMainWindow::subRigSelection(const PubSubName &sd, bool state)
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

void RSMainWindow::subRigControlFreq(const Frequency &lFreq, QString mode)
{
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QStringList qsl = rigCache.getSelectedLoggers(subRigSelected);
    if (qsl.count())
    {
        QString loggerUuid = qsl[0];

        st->addMember( loggerUuid, rpcConstants::loggerUuid );

        QString selc = rigCache.getSelectedContest(subRigSelected, loggerUuid);

        st->addMember(selc, rpcConstants::selected);
        st->addMember( lFreq.str(), rpcConstants::rigControlLogFreq );
        st->addMember( mode, rpcConstants::rigControlLogMode );

        rpc.getCallArgs() ->addParam( st );

        rpc.queueCall( subRigSelected);
    }
}


