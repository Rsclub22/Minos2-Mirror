#include "base_pch.h"
#include <QScrollArea>
#include "MinosLoggerEvents.h"

#include "ContestApp.h"
#include "MatchThread.h"
#include "BandList.h"
#include "tqsoeditdlg.h"
#include "tentryoptionsform.h"

#include "SendRPCDM.h"
#include "tlogcontainer.h"
#include "focuswatcher.h"
#include "htmldelegate.h"
#include "enqdlg.h"
#include "MatchTreeFrame.h"
#include "rigmemdialog.h"
#include "rigutils.h"
#include "LoggerContest.h"

#include "ScreenConfigFile.h"
#include "ScreenConfigElement.h"
#include "ScreenConfigRow.h"
#include "ScreenConfig.h"

#include "MatchArchiveFrame.h"
#include "MatchOtherFrame.h"
#include "MatchThisFrame.h"
#include "minossplitter.h"
#include "qsologframe.h"
#include "rigcontrolframe.h"
#include "rotcontrolframe.h"
#include "RotPresets.h"
#include "ChatFrame.h"
#include "clusterclientframe.h"
#include "bandmapclientframe.h"
#include "delayedaction.h"

#include "ContestPageControl.h"

#include "tsinglelogframe.h"
#include "ui_tsinglelogframe.h"

void TSingleLogFrame::buildFrame()
{
    createScreenComponents();

    buildScreenLayout();

    OtherMatchTreeFW = new FocusWatcher(otherMatchFrame->getTreeView());
    connect(OtherMatchTreeFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(onOtherMatchTreeFocused(QObject *, bool, QFocusEvent *)));
    ArchiveMatchTreeFW = new FocusWatcher(archiveMatchFrame->getTreeView());
    connect(ArchiveMatchTreeFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(onArchiveTreeFocused(QObject *, bool, QFocusEvent *)));

    restoreColumns();

    connect(&MinosLoggerEvents::mle, SIGNAL(ContestPageChanged()), this, SLOT(on_ContestPageChanged()));

    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(NextContactDetailsTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(PublishTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(HideTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(MakeEntry(BaseContestLog*)), this, SLOT(on_MakeEntry(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)), this, SLOT(on_AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterLogContact(BaseContestLog *)), this, SLOT(on_AfterLogContact(BaseContestLog *)));
    connect(&MinosLoggerEvents::mle, SIGNAL(setMemory(BaseContestLog *, QString, QString)), this, SLOT(on_SetMemory(BaseContestLog *, QString, QString)));
    // from cluster frame or bandmap frame
    connect(&MinosLoggerEvents::mle, SIGNAL(DxSpotToMemory(BaseContestLog *, memoryData::memData)), this, SLOT(on_dxSpotToMemory(BaseContestLog *, memoryData::memData)));
    connect(&MinosLoggerEvents::mle, SIGNAL(MatchStarting(BaseContestLog*)), this, SLOT(on_MatchStarting(BaseContestLog*)));

    connect(&MinosLoggerEvents::mle, SIGNAL(ColumnsChanged()), this, SLOT(onColumnsChanged()));
    connect(&MinosLoggerEvents::mle, SIGNAL(NextUnfilled(BaseContestLog*)), this, SLOT(on_NextUnfilled(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(GoToSerial(BaseContestLog*)), this, SLOT(on_GoToSerial(BaseContestLog*)));

    connect(&MinosLoggerEvents::mle, SIGNAL(XferPressed(BaseContestLog *, QString)), this, SLOT(on_XferPressed(BaseContestLog *, QString)));
    connect(thisMatchFrame, SIGNAL(editContact(QSharedPointer<BaseContact>)), this, SLOT(EditContact(QSharedPointer<BaseContact>)));

    connect(&MinosLoggerEvents::mle, SIGNAL(XferEnabled(bool, BaseContestLog *, QString)), GJVQSOLogFrame, SLOT(setXferEnabled(bool, BaseContestLog *, QString)));
    connect(&MinosLoggerEvents::mle, SIGNAL(MatchTreeSelected(MatchType , BaseContestLog *, QString, QItemSelection)),
            this, SLOT(MatchTreeSelected(MatchType, BaseContestLog *, QString, QItemSelection)));

    connect(&MinosLoggerEvents::mle, SIGNAL(doColumnChanges(BaseContestLog*)), this, SLOT(on_doColumnChanges(BaseContestLog*)));


    // RigControl Updates
    // From rig controller
    connect(LogContainer->sendDM, SIGNAL(setRadioLoaded()), this, SLOT(on_RadioLoaded()));
    connect(LogContainer->sendDM, SIGNAL(setRadioList()), this, SLOT(on_SetRadioList()));

    // To rig controller

    connect(FKHRigControlFrame, SIGNAL(radioDisconnected()), this, SLOT(invalidateCacheOnDisconnect()));
    connect(FKHRigControlFrame, SIGNAL(selectRadio(QString, Frequency, QString)), this, SLOT(sendSelectRadio(QString, Frequency, QString)));

    connect(FKHRigControlFrame, SIGNAL(sendFreqControl(Frequency)), this, SLOT(sendRadioFreq(Frequency)));
    connect(GJVQSOLogFrame, SIGNAL(sendFreqControl(Frequency)), this, SLOT(sendRadioFreq(Frequency)));
    connect(FKHRigControlFrame, SIGNAL(sendRitFreq(ShortFreq)), this, SLOT(sendRadioRitFreq(ShortFreq)));
    connect(FKHRigControlFrame, SIGNAL(sendVolumeToRadio(int)), this, SLOT(sendRadioVolume(int)));
    connect(FKHRigControlFrame, SIGNAL(ritStatus(bool)), this, SLOT(sendRadioRitStatus(bool)));
    connect(FKHRigControlFrame, SIGNAL(sendModeToControl(QString)), this, SLOT(sendRadioMode(QString)));
    connect(GJVQSOLogFrame, SIGNAL(sendModeControl(QString)), this , SLOT(sendRadioMode(QString)));

    connect(runButtonsFrame, SIGNAL(sendRunOnFlag(Frequency, QString, bool)), this, SLOT(sendRunOnFlag(Frequency, QString, bool)));
    connect(runButtonsFrame, SIGNAL(sendRunOffFreqFlag(Frequency, bool)), this, SLOT(sendRunOffFreqFlag(Frequency, bool)));


    // Rotator updates
    // From rotator controller
    connect(LogContainer->sendDM, SIGNAL(RotatorLoaded()), this, SLOT(on_RotatorLoaded()));
    connect(LogContainer->sendDM, SIGNAL(RotatorList()), this, SLOT(on_RotatorList()));

    // To rotator controller
    connect(FKHRotControlFrame, SIGNAL(sendRotator(rpcConstants::RotateDirection , int  )), this, SLOT(sendRotator(rpcConstants::RotateDirection , int  )));
    connect(rotPresets, SIGNAL(sendRotatorPreset(QString)), this, SLOT(sendRotatorPreset(QString)));
    connect(FKHRotControlFrame, SIGNAL(selectRotator(QString)), this, SLOT(sendSelectRotator(QString)));
    connect(FKHRotControlFrame, SIGNAL(selectRotator(QString)), rotPresets, SLOT(selectRotator(QString)));
    connect(rotPresets, SIGNAL(presetTurn(QString)), this, SLOT(presetTurn(QString)));
    connect(FKHRotControlFrame, SIGNAL(rotatorConnected(bool)), this, SLOT(on_rotatorConnected(bool)));

    // from cluster frame
    connect(&MinosLoggerEvents::mle, SIGNAL(DxSpotToLog(memoryData::memData)), this, SLOT(dxSpotToLog(memoryData::memData)));
    connect(&MinosLoggerEvents::mle, SIGNAL(ResendSpotsFromClusterCommand(resendFrameId, QString, int, QString)), this, SLOT(on_ResendSpotsFromClusterCommand(resendFrameId, QString, int, QString)));


    // to cluster server
    connect(GJVQSOLogFrame, SIGNAL(sendSpotToClusterServer(Frequency, QString, QString)), this, SLOT(on_SendSpotToClusterServer(Frequency, QString, QString)));

    // from cluster server
    connect(LogContainer->sendDM, SIGNAL(setClusterServerLoaded()),this, SLOT(on_clusterServerLoaded()));
    connect(LogContainer->sendDM, SIGNAL(setClusterState(QString)), this, SLOT(on_clusterServerState(QString)));
    connect(LogContainer->sendDM, SIGNAL(setClusterTXSpotEnableState(QString)), this, SLOT(on_setClusterTXSpotEnableState(QString)));


    // to bandmap
    connect(GJVQSOLogFrame, SIGNAL(bandmapMarkFreq(QString, Frequency, QString, QString, QString)),
            this, SLOT(on_BandmapMarkFreq(QString, Frequency, QString, QString, QString)));
    connect(GJVQSOLogFrame, SIGNAL(bandmapSaveFreq(QString, Frequency, QString, QString, QString)),
            this, SLOT(on_BandmapSaveFreq(QString, Frequency, QString, QString, QString)));
    //connect(FKHRigControlFrame, SIGNAL(sendCQFreq(QString, bool)), this, SLOT(on_SendCQFreq(QString, bool)));

    connect(FKHRigControlFrame, SIGNAL(radioIsConnected(bool)), this, SLOT(sendBandmapRadioIsConnected(bool)));
    connect(FKHRigControlFrame, SIGNAL(radioHasError(QString)), this, SLOT(sendBandmapRadioHasError(QString)));
    connect(LogContainer->sendDM, SIGNAL(setKeyerLoaded()), this, SLOT(on_KeyerLoaded()));


    connect( QSOTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));
    connect(QSOTable, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onQSOTable_doubleClicked(const QModelIndex &)));

    connect(LogContainer, SIGNAL(sendKeyerPlay( int )), this, SLOT(sendKeyerPlay(int)));
    connect(LogContainer, SIGNAL(sendKeyerRecord( int)), this, SLOT(sendKeyerRecord(int)));
    connect(LogContainer, SIGNAL(sendKeyerTone()), this, SLOT(sendKeyerTone()));
    connect(LogContainer, SIGNAL(sendKeyerTwoTone()), this, SLOT(sendKeyerTwoTone()));
    connect(LogContainer, SIGNAL(sendKeyerStop()), this, SLOT(sendKeyerStop()));


    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);
}

TSingleLogFrame::TSingleLogFrame(QWidget *parent, BaseContestLog * contest) :
    ContestPage(parent, contest),
    ui(new Ui::TSingleLogFrame),
    bandMapLoaded(false),
    rotatorLoaded(false),
    keyerLoaded(false),
    radioLoaded(false),
    lastStanzaCount( 0 )


{
    qRegisterMetaType< QSharedPointer<BaseContact> > ( "QSharedPointer<BaseContact>" );

    ui->setupUi(this);
}

void TSingleLogFrame::on_FontChanged()
{
    applyScreenLayout();
}
TSingleLogFrame::~TSingleLogFrame()
{
    delete ui;

    ui = nullptr;
    contest = nullptr;

    // we need to delete all the dependant ContestPage as well


    delete clusterControlFrame;
    delete wsjtxFrame;
}
void TSingleLogFrame::createScreenComponents()
{
    // create component frames, parentless
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    traceMsg("createScreenComponents for " + ct->name.getValue() + " uuid " + ct->uuid);

    QSOTable = new QTableView(this);
    QSOTable->setObjectName(QStringLiteral("QSOTable"));
    QSOTable->setFocusPolicy(Qt::ClickFocus);
    QSOTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QSOTable->setAlternatingRowColors(true);
    QSOTable->setSelectionMode(QAbstractItemView::SingleSelection);
    QSOTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    QSOTable->setWordWrap(false);
    QSOTable->horizontalHeader()->setHighlightSections(false);
    QSOTable->horizontalHeader()->setStretchLastSection(true);
    QSOTable->verticalHeader()->setVisible(false);
    QSOTable->setCornerButtonEnabled(false);
    QSOTable->verticalHeader()->setMinimumSectionSize(1);
    QSOTable->verticalHeader()->setDefaultSectionSize(1);

    QSOTable->horizontalHeader()->setMinimumSectionSize(10);

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0));
    qsoModel.delegate = delegate;
    qsoModel.initialise(contest);
    QSOTable->setModel(&qsoModel);

    // the order of the next two lines is critical
    QSOTable->setItemDelegate( delegate.data() );
    //QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


    QSOTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    QSOTable->setVisible(false);

    GJVQSOLogFrame = new QSOLogFrame(this);
    GJVQSOLogFrame->setObjectName(QStringLiteral("GJVQSOLogFrame"));

    GJVQSOLogFrame->setFrameShape(QFrame::NoFrame);
    GJVQSOLogFrame->setFrameShadow(QFrame::Plain);
    GJVQSOLogFrame->setLineWidth(2);
    GJVQSOLogFrame->setMidLineWidth(2);

    GJVQSOLogFrame->setVisible(false);
    GJVQSOLogFrame->setAsEdit(false, "Log");
    GJVQSOLogFrame->initialise( );

    FKHRigControlFrame = new RigControlFrame(this);
    FKHRigControlFrame->setObjectName(QStringLiteral("FKHRigControlFrame"));
    FKHRigControlFrame->setFrameShape(QFrame::StyledPanel);
    FKHRigControlFrame->setFrameShadow(QFrame::Raised);

    FKHRigControlFrame->setVisible(false);
    FKHRigControlFrame->setContest(contest);

    runButtonsFrame = new RunButtonsFrame(this);
    runButtonsFrame->setObjectName(QStringLiteral("runButtonsFrame"));
    runButtonsFrame->setVisible(false);
    runButtonsFrame->setRigControl(FKHRigControlFrame);
    runButtonsFrame->setContest(contest);

    FKHRotControlFrame = new RotControlFrame(this);

    FKHRotControlFrame->setObjectName(QStringLiteral("FKHRotControlFrame"));
    FKHRotControlFrame->setFrameShape(QFrame::StyledPanel);
    FKHRotControlFrame->setFrameShadow(QFrame::Raised);

    FKHRotControlFrame->setVisible(false);
    FKHRotControlFrame->setContest(contest);

    clusterControlFrame = new ClusterClientFrame(this);
    clusterControlFrame->setObjectName(QStringLiteral("ClusterControlFrame"));
    clusterControlFrame->setFrameShape(QFrame::StyledPanel);
    clusterControlFrame->setFrameShadow(QFrame::Raised);

    clusterControlFrame->setVisible(false);
    clusterControlFrame->setContest(contest);
    setClusterClientLoaded(false);

    bandmapControlFrame = new BandmapClientFrame(this);
    bandmapControlFrame->setObjectName(QStringLiteral("BandmapControlFrame"));
    bandmapControlFrame->setFrameShape(QFrame::StyledPanel);
    bandmapControlFrame->setFrameShadow(QFrame::Raised);

    bandmapControlFrame->setVisible(false);
    bandmapControlFrame->setContest(contest);
    setBandmapLoaded(false);


    rotPresets = new RotPresets(this);

    rotPresets->setObjectName(QStringLiteral("rotPresets"));
    rotPresets->setVisible(false);

    CribSheet = new QFrame(this);

    CribSheet->setObjectName(QStringLiteral("CribSheet"));

    CribSheet->setFrameShape(QFrame::NoFrame);
    CribSheet->setFrameShadow(QFrame::Plain);
    CribSheet->setLineWidth(1);
    CribSheet->setMidLineWidth(1);
    QVBoxLayout *verticalLayout_5 = new QVBoxLayout(CribSheet);
    verticalLayout_5->setSpacing(0);
    verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
    verticalLayout_5->setContentsMargins(10, 0, 5, 0);

    CurrentBandLabel = new QLabel(CribSheet);
    CurrentBandLabel->setObjectName(QStringLiteral("CurrentBandLabel"));
    CurrentBandLabel->setFrameShape(QFrame::NoFrame);
    CurrentBandLabel->setLineWidth(2);
    CurrentBandLabel->setMidLineWidth(2);
    CurrentBandLabel->setTextFormat(Qt::RichText);
    CurrentBandLabel->setWordWrap(true);

    CurrentBandLabel->setFrameStyle(QFrame::Panel | QFrame::Raised);

    verticalLayout_5->addWidget(CurrentBandLabel);

    NextContactDetailsLabel = new QLabel(CribSheet);
    NextContactDetailsLabel->setObjectName(QStringLiteral("NextContactDetailsLabel"));
    NextContactDetailsLabel->setFrameShape(QFrame::NoFrame);
    NextContactDetailsLabel->setLineWidth(2);
    NextContactDetailsLabel->setMidLineWidth(2);
    NextContactDetailsLabel->setTextFormat(Qt::RichText);
    NextContactDetailsLabel->setWordWrap(true);
    verticalLayout_5->addWidget(NextContactDetailsLabel);
    verticalLayout_5->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    CribSheet->setVisible(false);

    thisMatchFrame = new MatchThisFrame(this);

    thisMatchFrame->setObjectName(QStringLiteral("thisMatchFrame"));
    thisMatchFrame->setFrameShape(QFrame::StyledPanel);
    thisMatchFrame->setFrameShadow(QFrame::Raised);

    thisMatchFrame->setVisible(false);
    thisMatchFrame->initialise();
    thisMatchFrame->setBaseName("Log");

    otherMatchFrame = new MatchOtherFrame(this);

    otherMatchFrame->setObjectName(QStringLiteral("otherMatchFrame"));
    otherMatchFrame->setFrameShape(QFrame::StyledPanel);
    otherMatchFrame->setFrameShadow(QFrame::Raised);

    otherMatchFrame->setVisible(false);
    otherMatchFrame->initialise();
    otherMatchFrame->setBaseName("Log");

    archiveMatchFrame = new MatchArchiveFrame(this);

    archiveMatchFrame->setObjectName(QStringLiteral("archiveMatchFrame"));
    archiveMatchFrame->setFrameShape(QFrame::StyledPanel);
    archiveMatchFrame->setFrameShadow(QFrame::Raised);

    archiveMatchFrame->setVisible(false);
    archiveMatchFrame->initialise();
    archiveMatchFrame->setBaseName("Log");

    chatFrame = new ChatFrame(this);
    chatFrame->setObjectName(QStringLiteral("chatFrame"));
    chatFrame->setFrameShape(QFrame::StyledPanel);
    chatFrame->setFrameShadow(QFrame::Raised);

    chatFrame->setVisible(false);

    wsjtxFrame = new WsjtxFrame(this);
    wsjtxFrame->setObjectName(QStringLiteral("wsjtxFrame"));
    wsjtxFrame->setFrameShape(QFrame::StyledPanel);
    wsjtxFrame->setFrameShadow(QFrame::Raised);

    wsjtxFrame->setVisible(false);

}
void TSingleLogFrame::clearScreenLayout(bool clearAllTabs)
{
    // clear down the screen elements, but don't delete them (except for the aux frames) - they will be used to rebuild the screen
    // BUT on contest creation, the contest address may change, so clear the contest

    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    QString msg = ct->name.getValue() + " uuid " + ct->uuid;
    traceMsg("clearScreenLayout starts for " + msg);

    qsoModel.initialise(nullptr);

    FKHRigControlFrame->setContest(nullptr);
    runButtonsFrame->setContest(nullptr);
    FKHRotControlFrame->setContest(nullptr);
    rotPresets->setContest(nullptr);
    // CribSheet
    // NextContactDetailsLabel
    // CurrentBandLabel
    GJVQSOLogFrame->setContest(nullptr);
    thisMatchFrame->setContest(nullptr);
    otherMatchFrame->setContest(nullptr);
    archiveMatchFrame->setContest(nullptr);
    //chatFrame
    wsjtxFrame->setContest(nullptr);
    clusterControlFrame->setContest(nullptr);
    setClusterClientLoaded(false);
    bandmapControlFrame->setContest(nullptr);
    setBandmapLoaded(false);

    // we need to setContest(nullptr) on all aux frames
    MinosLoggerEvents::SendClearContestInFrame(ct);

    clusterControlFrame->setParent(this);
    clusterControlFrame->hide();

    bandmapControlFrame->setParent(this);
    bandmapControlFrame->hide();

    if (LogContainer->isLoggerClosing())
    {
        // do nothing more...
    }
    else
    {
        QSOTable->setParent(this);
        QSOTable->hide();

        FKHRigControlFrame->setParent(this);
        FKHRigControlFrame->hide();

        runButtonsFrame->setParent(this);
        runButtonsFrame->hide();

        FKHRotControlFrame->setParent(this);
        FKHRotControlFrame->hide();

        rotPresets->setParent(this);
        rotPresets->hide();

        CribSheet->setParent(this);
        CribSheet->hide();

        GJVQSOLogFrame->setParent(this);
        GJVQSOLogFrame->hide();

        thisMatchFrame->setParent(this);
        thisMatchFrame->hide();

        otherMatchFrame->setParent(this);
        otherMatchFrame->hide();

        archiveMatchFrame->setParent(this);
        archiveMatchFrame->hide();

        chatFrame->setParent(this);
        chatFrame->hide();

        wsjtxFrame->setParent(this);
        wsjtxFrame->hide();

        if (clearAllTabs)
        {
            for (auto cpc = LogContainer->contestPageControls.begin(); cpc != LogContainer->contestPageControls.end(); cpc++)
            {
                if ((*cpc) == nullptr)
                {
                    continue;
                }
                for  (auto cp = (*cpc)->pages.begin(); cp != (*cpc)->pages.end(); cp++)
                {
                    if (cp.key() == contest)
                    {
                        cp.value()->clearScreen();
                        if (cp.value() != this)
                        {
                            cp.value()->deleteLater();
                        }
                        (*cpc)->pages.remove(cp.key());
                        break;
                    }
                }
            }
        }
    }
    traceMsg("clearScreenLayout complete for " + msg);
}
void TSingleLogFrame::applyScreenLayout()
{
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    if (!ct)
        return;
    traceMsg("applyScreenLayout for " + ct->name.getValue() + " uuid " + ct->uuid);
    QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    clearScreenLayout(true);
    buildScreenLayout();
    QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

QString TSingleLogFrame::getCurScreenLayout() const
{
    return curScreenLayout;
}

void TSingleLogFrame::setCurScreenLayout(const QString &value)
{
    curScreenLayout = value;
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
    ct->screenLayout.setValue(value);
    ct->commonSave(false);
}
void TSingleLogFrame::buildRow(SCRow &scrow, int &auxInstance, MinosSplitter *splitterParent)
{
    // This builds the dependant ContestPage (including the one we derive from)
    if (scrow.elements.count())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

        // insert horizontal splitter in splitterParent
        MinosSplitter *hs = new MinosSplitter();
        hs->setObjectName("row" + QString::number(rowSplitters.size()) + "splitter");
        hs->setOrientation(Qt::Horizontal);
        hs->setChildrenCollapsible(false);
        rowSplitters.push_back(hs);

        for (auto &scele: scrow.elements)
        {
            //SCElement scele = scrow.elements[srele];
            SCType type = scele.type;
            if (type == sctNone)
                continue;

            QScrollArea *elementScrollArea = nullptr;
            if (type != sctLog
                    && type != sctThisMatch
                    && type != sctOtherMatch
                    && type != sctArchiveMatch
                    && type != sctSplit
                    && type != sctCluster
                    && type != sctWsjtx
                    && type != sctBandmap
                    )
            {
                elementScrollArea = new QScrollArea();
                elementScrollArea->setWidgetResizable(true);
                elementScrollArea->setFocusPolicy(Qt::NoFocus);
                hs->addWidget(elementScrollArea);
            }

            // insert correct widget type in horizontal splitter

            switch (type)
            {
                case sctNone:
                case sctMainScreen:
                case sctScreen:
                {
                    break;
                }
                case sctAux:
                {
                    StackedInfoFrame *f = new StackedInfoFrame(elementScrollArea, auxInstance++);

                    f->setCurrentFrameType(StackedInfoFrame::getTrAuxTypeString(scele.auxType));
                    f->setContest(ct);
                    elementScrollArea->setWidget(f);
                    f->setVisible(true);
                    break;
                }
                case sctLog:
                {
                    QSOTable->setParent(hs);
                    hs->addWidget(QSOTable);
                    QSOTable->setVisible(true);
                    break;
                }
                case sctRigControl:
                {
                    elementScrollArea->setWidget(FKHRigControlFrame);
                    // don't set contest here
                    break;
                }
                case sctRunButtons:
                {
                    elementScrollArea->setWidget(runButtonsFrame);
                    runButtonsFrame->setContest(ct);
                    break;
                }
                case sctRotControl:
                {
                    elementScrollArea->setWidget(FKHRotControlFrame);
                    // don't set contest here
                    break;
                }
                case sctRotPresets:
                {
                    elementScrollArea->setWidget(rotPresets);
                    rotPresets->setContest(ct);
                    break;
                }
                case sctQSOEdit:
                {
                    elementScrollArea->setWidget(GJVQSOLogFrame);
                    GJVQSOLogFrame->setContest(ct);
                    GJVQSOLogFrame->setVisible(true);
                    break;
                }
                case sctNextQSODetails:
                {
                    elementScrollArea->setWidget(CribSheet);
                    CribSheet->setVisible(true);
                    break;
                }
                case sctThisMatch:
                {
                    hs->addWidget(thisMatchFrame);
                    thisMatchFrame->setVisible(true);
                    thisMatchFrame->setContest(ct);
                    break;
                }
                case sctOtherMatch:
                {
                    hs->addWidget(otherMatchFrame);
                    otherMatchFrame->setVisible(true);
                    otherMatchFrame->setContest(ct);
                    break;
                }
                case sctArchiveMatch:
                {
                    hs->addWidget(archiveMatchFrame);
                    archiveMatchFrame->setVisible(true);
                    archiveMatchFrame->setContest(ct);
                    break;
                }
                case sctChat:
                {
                    elementScrollArea->setWidget(chatFrame);
                    chatFrame->setVisible(true);
                    break;
                }
                case sctCluster:
                {
                    hs->addWidget(clusterControlFrame);
                    clusterControlFrame->setVisible(true);
                    clusterControlFrame->setContest(ct);
                    setClusterClientLoaded(true);
                    break;

                }
                case sctBandmap:
                {
                    hs->addWidget(bandmapControlFrame);
                    bandmapControlFrame->setVisible(true);
                    bandmapControlFrame->setContest(ct);
                    setBandmapLoaded(true);
                    break;

                }
                case sctWsjtx:
                {
                    hs->addWidget(wsjtxFrame);
                    wsjtxFrame->setVisible(true);
                    // don't set contest here
                    break;
                }
                case sctSplit:
                {
                    MinosSplitter *vs = new MinosSplitter();
                    vs->setObjectName("splitRow" + QString::number(rowSplitters.size()) + "splitter");
                    vs->setOrientation(Qt::Vertical);
                    vs->setChildrenCollapsible(false);
                    rowSplitters.push_back(vs);

                    for (auto &srow: scele.rows)
                    {
                        buildRow(srow, auxInstance, vs);
                    }

                    hs->addWidget(vs);
                    break;
                }
            }
        }
        splitterParent->addWidget(hs);
    }

}
void TSingleLogFrame::buildScreen(SCScreen &s, int t, int &auxInstance)
{
    // we need to add this contest page to the relevant contestPageControl
    // as a new tab

    // How do we make sure that ALL contests are in ALL page controls, even when
    // they have no such screen?

    ContestPage *cp = this;
    if (t > 0)
    {
        for (int i = 0; i <= t; i++)
        {
            if (LogContainer->contestPageControls.count() <= i
               || LogContainer->contestPageControls[i] == nullptr)
            {
                ContestPageControl *cpc = new ContestPageControl();
                cpc->setInstance(i);
                if (i < LogContainer->contestPageControls.count())
                {
                    LogContainer->contestPageControls[i] = cpc;
                }
                else
                {
                    LogContainer->contestPageControls.append(cpc);
                }
                cpc->setWindowFlags(/*Qt::Tool |*/ Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                if (i != 0)
                {
                    cpc->setAttribute(Qt::WA_ShowWithoutActivating);
                }
                cpc->show();
            }
        }
        cp = new ContestPage(nullptr, contest);
        QString n = QString("contestpage%1").arg(t);
        cp->setObjectName(n);
        cp->setStyleSheet(QString(" #%1 { border: 2px solid blue; }").arg(n));
    }
    ContestPageControl *cpc = LogContainer->contestPageControls[t];
    cpc->pages[contest] = cp;
    cp->pageNo = t;
    cp->buildScreen(this, s, auxInstance);

    bool temp = TContestApp::getContestApp() ->suppressWritePreload;
    TContestApp::getContestApp() ->suppressWritePreload = true;
    QString sname  = s.name;
    if (t == 0 || sname.isEmpty())
    {
        sname = ExtractFileName( contest->cfileName );
    }
    LogContainer->contestPageControls[t]->addTab(cp, sname);
    TContestApp::getContestApp() ->suppressWritePreload = temp;
}
void TSingleLogFrame::buildScreenLayout()
{
    ScreenConfigFile &scf = ScreenConfigFile::getScreenConfigFile(this);

    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

    QString curConfigName;
    if (ct->isReadOnly())
    {
        MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpProtectedLayout, curConfigName );
    }
    else
    {
        curConfigName = ct->screenLayout.getValue();
    }
    traceMsg("buildScreenLayout for " + ct->name.getValue() + " uuid " + ct->uuid + " to layout " + curConfigName);
    if (curConfigName.isEmpty() || !scf.configs.contains(curConfigName))
    {
        curConfigName = defaultLayoutName();
        ct->screenLayout.setValue(curConfigName);
    }
    curScreenLayout = curConfigName;

    SC sc = scf.configs[curConfigName];

    int auxInstance = 0;
    int t = 0;
    for (auto &s: sc.baseElement->screens)
    {
        buildScreen(s, t++, auxInstance);
    }
    qsoModel.initialise(contest);
    QSOTable->setModel(&qsoModel);

    // ALWAYS link the wsjt frame to the contest; then we can log
    // even without showing it
    wsjtxFrame->setContest(ct);
    FKHRigControlFrame->setContest(ct);
    FKHRotControlFrame->setContest(ct);
}


void TSingleLogFrame::keyPressEvent( QKeyEvent* event )
{
    // each dependant ContestPage also needs this
    GJVQSOLogFrame->doKeyPressEvent(event);
}

QString TSingleLogFrame::makeEntry( bool saveMinos )
{
   LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>( contest );
   if ( !ct )
   {
      return "";
   }

   TEntryOptionsForm EntryDlg( this, QSharedPointer<ContestDetailsTransferObject>(), ct, saveMinos  );
   if ( saveMinos )
   {
      EntryDlg.setWindowTitle(tr("Save imported log as a .minos file"));
   }
   if ( EntryDlg.exec() == QDialog::Accepted )
   {
      ct->commonSave( false );
      QString expName = EntryDlg.doFileSave( );
      return expName;
   }
   return "";
}

void TSingleLogFrame::closeContest()
{
    if ( TContestApp::getContestApp() )
    {
       FKHRigControlFrame->closeContest();          // this disconnects rig on last closing contest
       FKHRotControlFrame->closeContest();
       GJVQSOLogFrame->closeContest();
       if (contest)
       {
            RPCPubSub::publish( rpcConstants::monitorLogCategory, contest->publishedName, QString::number( 0 ), psRevoked );
       }

       clearScreenLayout(false);
       TContestApp::getContestApp() ->closeFile( contest );
       qsoModel.initialise(nullptr);

       contest = nullptr;
    }
}

void TSingleLogFrame::addAllQSOsToBandmap()
{
    for ( auto const &c: contest->ctList )
    {
        QSharedPointer<BaseContact> cct = c.wt;
        // Extract comments for "Remarks" section
        //cct->addReg1TestComment( remarks );

        if ( cct->contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
           continue;

        bandmapControlFrame->on_AfterLogContact(contest, cct);
    }
}
void TSingleLogFrame::restoreColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("QSOTable/state").toByteArray();
    QSOTable->horizontalHeader()->restoreState(state);

    QSOTable->horizontalHeader()->setMinimumSectionSize(10);

// these now subscribe for themselves
//    thisMatchFrame->restoreColumns();
//    otherMatchFrame->restoreColumns();
//    archiveMatchFrame->restoreColumns();

    QFont cf = QApplication::font();
    QSOTable->horizontalHeader()->setFont(cf);
    columnsChanged = false;

}

void TSingleLogFrame::showQSOs()
{
    ScreenContact *p = GJVQSOLogFrame->getPartialContact();
    GJVQSOLogFrame->setPartialContact(nullptr);

   NextContactDetailsTimerTimer( );

   restoreColumns();
   columnsChanged = false;

   GJVQSOLogFrame->clearCurrentField();
   GJVQSOLogFrame->startNextEntry();

   GJVQSOLogFrame->killPartial();
   GJVQSOLogFrame->setPartialContact(p);

}
void TSingleLogFrame::on_ContestPageChanged ()
{
    if (!ui)
        return;

    if ( this != LogContainer->getCurrentLogFrame() )
    {

       GJVQSOLogFrame->savePartial();  // we kill it on (re) entry, so not needed
       return ;
    }

    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    if (ct)
    {
        traceMsg("on_ContestPageChanged to " + ct->name.getValue() + " uuid " + ct->uuid);
    }
    else
    {
        traceMsg("on_ContestPageChanged, no contest set in page");
    }
    TContestApp::getContestApp() ->setCurrentContest( ct );

    MinosLoggerEvents::SendContestShownChanged();

    if ( columnsChanged )
    {
        MinosLoggerEvents::SendDoColumnChanges(ct);             // this does a restorePartial in showQSOs
        columnsChanged = false;
    }

//    if (splittersChanged)
//    {
//        MinosLoggerEvents::SendDoSplitterChanges(ct);
//        splittersChanged = false;
//    }

    refreshMults();

    GJVQSOLogFrame->selectField(nullptr);
    GJVQSOLogFrame->logTabChanged();

    MinosLoggerEvents::SendShowOperators();

    LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
    LogContainer->sendDM->invalidateRotatorCache(ct->antennaName.getValue());


    LogContainer->sendDM->notifyRigChanges();
    LogContainer->sendDM->notifyRotChanges();

    FKHRigControlFrame->on_ContestPageChanged();
    FKHRotControlFrame->on_ContestPageChanged();

    updateQSODisplay();

   //QHeaderView::ResizeMode rm =  QSOTable->verticalHeader()->sectionResizeMode(1);
   // default seems to be 0, Interactive
    QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    update();   // this queues a repaint
}
void TSingleLogFrame::on_doColumnChanges(BaseContestLog *b)
{
    if (b == contest)
    {
        showQSOs();             // this does a restorePartial
    }
}

void TSingleLogFrame::NextContactDetailsTimerTimer( )
{
    if ( contest )
    {
        QColor bic = Qt::black;
        QString cb = contest->currentBand.getValue().trimmed();

        BandList &blist = BandList::getBandList();
        QSharedPointer<BandInfo>  bi;
        bool bandOK = blist.findBand(cb, bi);
        if (bandOK)
        {
            cb = bi->uk;
            bic = bi->bandColour;
        }
        else
        {
            bic = Qt::red;
        }

        //we want to put a line across, and colour the bands - need a map of band->colour
        // ideally we want it configurable...

        CurrentBandLabel->setText( HtmlFontColour(bic) + "<b><center><nobr><p><big><h1>" + cb);

        if ( contest->isReadOnly() )
        {
            NextContactDetailsLabel->setText( "<b><center><nobr><p><big><h1>"
                                                  + contest->mycall.getFullCall() + "<br>"
                                                  + contest->myloc.getLoc() + "<br>"
                                                  + contest->location.getValue());
        }
        else
        {
            QString snBuff = QString("%1").arg( contest->maxSerial + 1, 3, 10, QChar('0') );
            QString locBuff;
            if (contest->location.getValue().size())
            {
                locBuff = "<br>" + contest->location.getValue();
            }
            NextContactDetailsLabel->setText( "<b><center><nobr><p><big><h1>"
                                                  + contest->mycall.getFullCall() + "<br>"
                                                  + snBuff + "<br>"
                                                  + contest->myloc.getLoc()
                                                  + locBuff);
        }
    }
}
void TSingleLogFrame::PublishTimerTimer(  )
{
   LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>( contest );
   if ( ct && ct->isMinosFile() && !ct->isReadOnly())
   {
      int stanzaCount = contest->getCtStanzaCount();
      if ( lastStanzaCount != stanzaCount )
      {
         // publish this contest details - what to use?
         // category LoggerContestLog
         // name filename(?)
         // value stanzaCount
          QString name = contest->name.getValue();
          QString band = contest->contestBands.getValue();

          QString cell = QString::number( stanzaCount ) + ";[" + band + "] " + name;

         RPCPubSub::publish( rpcConstants::monitorLogCategory, contest->publishedName, cell, psPublished );
         lastStanzaCount = stanzaCount;
      }
   }
}
void TSingleLogFrame::HideTimerTimer(  )
{
    if (!contest)
        return;


}

void TSingleLogFrame::updateQSODisplay()
{
   GJVQSOLogFrame->updateQSODisplay();
}

void TSingleLogFrame::onOtherMatchTreeFocused(QObject *, bool in, QFocusEvent * )
{
    if (!in)
    {
        archiveMatchFrame->getTreeView()->viewport()->update();
        otherMatchFrame->getTreeView()->viewport()->update();
        return;
    }

    xferTree = otherMatchFrame;
    otherMatchFrame->setCurrentModel(true);
    archiveMatchFrame->setCurrentModel(false);

    archiveMatchFrame->getTreeView()->viewport()->update();
    otherMatchFrame->getTreeView()->viewport()->update();
}

void TSingleLogFrame::onArchiveTreeFocused(QObject *, bool in, QFocusEvent * )
{
    if (!in)
    {
        archiveMatchFrame->getTreeView()->viewport()->update();
        otherMatchFrame->getTreeView()->viewport()->update();
        return;
    }

    xferTree = archiveMatchFrame;
    archiveMatchFrame->setCurrentModel( true);
    otherMatchFrame->setCurrentModel(false);

    archiveMatchFrame->getTreeView()->viewport()->update();
    otherMatchFrame->getTreeView()->viewport()->update();
}
void TSingleLogFrame::on_MatchStarting(BaseContestLog *ct)
{
      // clear down match trees
    if (contest == ct)
    {
      xferTree = nullptr;

      thisMatchFrame->treeClickIndex = QModelIndex();
      otherMatchFrame->treeClickIndex = QModelIndex();
      archiveMatchFrame->treeClickIndex = QModelIndex();

      GJVQSOLogFrame->setXferEnabled(false, contest, "Log");
    }
}

MatchTreeItem * TSingleLogFrame::getXferItem()
{
   // transfer from current match

   // copy relevant parts of match contact to screen contact
   if ( archiveMatchFrame->treeClickIndex.isValid() && ( xferTree == nullptr ||  archiveMatchFrame == xferTree ) )
   {
      MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * >(archiveMatchFrame->treeClickIndex.internalPointer());

      return MatchTreeIndex;

   }
   else
   {
      if ( otherMatchFrame->treeClickIndex.isValid() && ( xferTree == nullptr || otherMatchFrame == xferTree) )
      {
         MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * > (otherMatchFrame->treeClickIndex.internalPointer());

         return  MatchTreeIndex;
      }
   }
   return nullptr;
}
void TSingleLogFrame::on_XferPressed(BaseContestLog *c, QString basename)
{
   // transfer from current match
   if (!contest || contest->isReadOnly() || c != contest || basename != "Log" )
      return ;

   MatchTreeItem *mi = getXferItem();

   transferDetails(mi);
}
void TSingleLogFrame::MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, const QItemSelection &/*selected*/)
{
    if (contest == c && basename == "Log")
    {
        switch (m)
        {
        case ThisMatch:
            //xferTree =  thisMatchFrame;
            break;

        case OtherMatch:
            xferTree = otherMatchFrame;
            break;

        case ArchiveMatch:
            xferTree = archiveMatchFrame;
            break;
        }
    }
}

//==============================================================================
void TSingleLogFrame::transferDetails(MatchTreeItem *MatchTreeIndex )
{
    if ( !contest || !MatchTreeIndex  )
    {
       return ;
    }
   // needs to be transferred into QSOLogFrame.cpp
   QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();

   if (mc)
   {
       QSharedPointer<BaseContact> bct = mc->getBaseContact();

       if ( bct )
       {
          BaseContestLog *matct = mc->getContactLog();
          GJVQSOLogFrame->transferDetails( bct, matct );
       }
       else
       {
           ListContact *lct = mc->getListContact();
           if (lct)
           {
               ContactList *matct = mc->getContactList();
               GJVQSOLogFrame->transferDetails( lct, matct );
           }
       }
   }
}

void TSingleLogFrame::dxSpotToLog(memoryData::memData m )
{
    transferDetails(m);
}


void TSingleLogFrame::on_SendSpotToClusterServer(Frequency freq, QString callsign, QString loc)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendSpotToClusterServer(freq, callsign, loc);
    }
}

void TSingleLogFrame::on_ResendSpotsFromClusterCommand(resendFrameId frameId, QString cmd, int bandmask, QString uuid)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRequestSpotsResentFromClusterServer(frameId, cmd, bandmask, uuid );
    }
}




void TSingleLogFrame::transferDetails(memoryData::memData &m )
{
    if ( !contest  )
    {
       return ;
    }

    GJVQSOLogFrame->transferDetails( m.callsign, m.locator, m.exchange, m.fromBandmapOrMemory );
    FKHRigControlFrame->transferDetails(m);
}
void TSingleLogFrame::getDetails(memoryData::memData &m)
{
    FKHRigControlFrame->getDetails(m);
}
void TSingleLogFrame::getCurrentDetails(memoryData::memData &m)
{
    FKHRigControlFrame->getRigDetails(m);
    FKHRotControlFrame->getRotDetails(m);
}
//---------------------------------------------------------------------------

void TSingleLogFrame::QSOTreeSelectContact( QSharedPointer<BaseContact> lct )
{
   if (lct)
   {
      EditContact( lct );
   }
}
void TSingleLogFrame::onQSOTable_doubleClicked(const QModelIndex &index)
{
    QSOTreeSelectContact(contest->pcontactAt( index.row() ));
}
void TSingleLogFrame::EditContact( QSharedPointer<BaseContact> lct )
{
   TQSOEditDlg qdlg( this, false );
   qdlg.selectContact( contest, lct );

   trace(QString("TSingleLogFrame::EditContact %1").arg(lct->cs.getFullCall()));

   qdlg.exec();

   trace(QString("TSingleLogFrame::EditContact finished %1").arg(lct->cs.getFullCall()));

   contest->scanContest();

   GJVQSOLogFrame->refreshOps();
   refreshMults();
   GJVQSOLogFrame->startNextEntry();

}
//---------------------------------------------------------------------------
ScreenContact &TSingleLogFrame::getScreenEntry()
{
    GJVQSOLogFrame->getScreenEntry();

    GJVQSOLogFrame->calcLoc();

    return GJVQSOLogFrame->screenContact;
}
//---------------------------------------------------------------------------
int TSingleLogFrame::getBearingFrmQSOLog()
{
    return FKHRotControlFrame->getAngle(GJVQSOLogFrame->getBearing());
}

int TSingleLogFrame::getCurrentBearing()
{
    return FKHRotControlFrame->getCurrentBearing();
}
//---------------------------------------------------------------------------

void TSingleLogFrame::on_MakeEntry(BaseContestLog *ct)
{
    if (ct == contest)
    {
       makeEntry( false );
    }
}
void TSingleLogFrame::on_AfterSelectContact( QSharedPointer<BaseContact>lct, BaseContestLog *ct)
{
    if (ct == contest && !lct)
    {
        // use a lambda on a short timer as when contest is first opened, it doesn't actually scroll
        delayedAction(this, [=]()
        {
            // NB a lambda function
            QSOTable->scrollToBottom();
            int row = QSOTable->model()->rowCount() - 1;
            if (row >= 0)
            {
                QModelIndex oldIndex = QSOTable->currentIndex();
                if (oldIndex.row() != row)
                {
                    QModelIndex index = QSOTable->model()->index( row, 0 );
                    QSOTable->setCurrentIndex(index);
                }
            }
        }
        );
    }
}
void TSingleLogFrame::on_AfterLogContact( BaseContestLog *ct)
{
      if (ct == contest)
      {
         contest->scanContest();
         updateTrees();
         NextContactDetailsTimerTimer( );
      }
}
void TSingleLogFrame::refreshMults()
{
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
    MinosLoggerEvents::sendRefreshStackMults(ct);
}

void TSingleLogFrame::updateTrees()
{
   qsoModel.reset();
   refreshMults();
}
bool TSingleLogFrame::getStanza( unsigned int stanza, QString &stanzaData )
{
   return contest->getStanza( stanza, stanzaData );
}
void TSingleLogFrame::on_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;

    state = QSOTable->horizontalHeader()->saveState();
    settings.setValue("QSOTable/state", state);

    MinosLoggerEvents::SendColumnsChanged();
}
void TSingleLogFrame::onColumnsChanged()
{
    columnsChanged = true;
}
void TSingleLogFrame::goNextUnfilled()
{
   QSharedPointer<BaseContact> nuc = contest->findNextUnfilledContact( );
   if ( nuc )
   {
      TQSOEditDlg qdlg(this, true );
      qdlg.setContest( contest );
      qdlg.setFirstContact( nuc );
      qdlg.exec();
      contest->scanContest();
      refreshMults();
      GJVQSOLogFrame->startNextEntry();
   }
   else
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("No unfilled contacts") );
   }

}
void TSingleLogFrame::on_NextUnfilled(BaseContestLog *ct)
{
    if (ct == contest)
    {
       goNextUnfilled();
    }
}

void TSingleLogFrame::goSerial( )
{
    static int serial = 0;
    do
    {
       if ( serial == -1 )
          serial = 0;
       if ( !enquireDialog( this, tr("Please give serial wanted"), serial ) )
          return ;
    }
    while ( serial == -1 );

    QSharedPointer<BaseContact> cfu;
    for ( auto const &c: contest->ctList )
    {
        bool ok;
        int s = c.wt->serials.getValue().toInt(&ok );
       if ( ok && serial == s )
       {
          cfu = c.wt;
          break;
       }
    }

    if ( cfu )
    {
       EditContact( cfu );
    }
    else
       MinosParameters::getMinosParameters() ->mshowMessage( tr("Serial number %1 not found").arg(serial) );
}

void TSingleLogFrame::on_GoToSerial(BaseContestLog *ct)
{
    if (ct == contest)
    {
       goSerial();
    }
}
//---------------------------------------------------------------------------

// Memory

void TSingleLogFrame::on_SetMemory(BaseContestLog *c, QString call, QString loc)
{
    if (contest == c)
    {
        int n = -1;
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        int mcount = ct->rigMemories.size();
        for (int i = 0; i <= mcount; i ++)
        {
            memoryData::memData m = ct->getRigMemoryData(i);

            if ( m.callsign == memDefData::DEFAULT_CALLSIGN)
            {
                n = i;
                break;
            }
        }

        if (n == -1)
        {
            mShowMessage("Panic", this);
            return;
        }
        memoryData::memData logData;
        getDetails(logData);
        logData.callsign = call;
        logData.locator = loc;
        logData.time = QTime::currentTime().toString("HH:mm");

        RigMemDialog memDialog(this);
        memDialog.setLogData(&logData, n, ct);
        memDialog.setWindowTitle(QString("M%1 - Write").arg(QString::number(n + 1)));
       if ( memDialog.exec() == QDialog::Accepted)
       {
           ct->saveRigMemory(n, logData);

           MinosLoggerEvents::sendUpdateMemories(ct);
       }
    }
}



// send to memory from DXCluster frame or Bandmapframe

void TSingleLogFrame::on_dxSpotToMemory(BaseContestLog *c, memoryData::memData dxData)
{

    if (contest == c)
    {


        int n = -1;
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

        int mcount = ct->rigMemories.size();
        for (int i = 0; i <= mcount; i ++)  // <= - extra one gets blank
        {
            memoryData::memData m = ct->getRigMemoryData(i);

            if ( m.callsign == memDefData::DEFAULT_CALLSIGN)
            {
                n = i;
                break;
            }
        }

        if (n == -1)
        {
            mShowMessage("Panic", this);
            return;
        }

        ct->saveRigMemory(n, dxData);

        MinosLoggerEvents::sendUpdateMemories(ct);
    }
}




//---------------------------------------------------------------------------
void TSingleLogFrame::on_KeyerLoaded()
{
   keyerLoaded = true;
   GJVQSOLogFrame->setKeyerLoaded();
}
bool TSingleLogFrame::isKeyerLoaded()
{
   return keyerLoaded;
}


void TSingleLogFrame::sendKeyerPlay( int fno )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerPlay(this, fno);
}

void TSingleLogFrame::sendKeyerRecord( int fno )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerRecord(this, fno);
}

//void TSingleLogFrame::sendBandMap( QString freq, QString call, QString utc, QString loc, QString qth )
//{
//    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
//        LogContainer->sendDM->sendBandMap(this, freq, call, utc, loc, qth);
//}


void TSingleLogFrame::on_BandmapMarkFreq(QString cs, Frequency freq, QString loc, QString brg, QString exchange)
{
    bandmapControlFrame->setBandmapMarkFreq(cs, freq, loc, brg, exchange);
}


void TSingleLogFrame::on_BandmapSaveFreq(QString cs, Frequency freq, QString loc, QString brg, QString exchange)
{
    bandmapControlFrame->setBandmapSaveFreq(cs, freq, loc, brg, exchange);
}

void TSingleLogFrame::sendBandmapRadioIsConnected(bool state)
{
    bandmapControlFrame->setBandmapRadioIsConnect(state);
}

void TSingleLogFrame::sendBandmapRadioHasError(QString error)
{
    bandmapControlFrame->setBandmapRadioHasError(error);
}

//void TSingleLogFrame::on_SendCQFreq(QString runFreq, bool showMarker)
//{
//    bandmapControlFrame->setCQFreq(runFreq, showMarker);
//}

void TSingleLogFrame::sendRunOnFlag(Frequency runFreq, QString mode, bool runModeOn)
{
    GJVQSOLogFrame->setRunOnFlag(runModeOn);
    bandmapControlFrame->setRunOnFlag(runFreq, mode, runModeOn);
}

void TSingleLogFrame::sendRunOffFreqFlag(Frequency runFreq, bool offRunFreq)
{
    GJVQSOLogFrame->setRunOffFreqFlag(offRunFreq);
    bandmapControlFrame->setRunOffFreqFlag(runFreq, offRunFreq);
}

void TSingleLogFrame::on_ZoomMap(bool dir)
{
    bandmapControlFrame->updateZoom(dir);
}

void TSingleLogFrame::sendKeyerTone()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerTone(this);
}

void TSingleLogFrame::sendKeyerTwoTone()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerTwoTone(this);
}

void TSingleLogFrame::sendKeyerStop()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendKeyerStop(this);
}


//---------------------------------------------------------------------------

// Bandmap

void TSingleLogFrame::setBandmapLoaded(bool loaded)
{
   bandMapLoaded = loaded;
   GJVQSOLogFrame->setBandMapLoaded(loaded);
}

bool TSingleLogFrame::isBandMapLoaded()
{
   return bandMapLoaded;
}

bool TSingleLogFrame::getTuneAddBandMapSetting()
{
    bool state = false;
    if (bandmapControlFrame)
    {
        state = bandmapControlFrame->readTuneAddBandMapSetting();
    }

    return state;
}

void TSingleLogFrame::setTuneAddBandMapSetting(bool state)
{
    if (bandmapControlFrame)
    {
        bandmapControlFrame->saveTuneAddBandMapSetting(state);
    }
}

//---------------------------------------------------------------------------

// Cluster


void TSingleLogFrame::on_clusterServerLoaded()
{
    setClusterServerLoaded(true);
}

void TSingleLogFrame::setClusterServerLoaded(bool loaded)
{
   clusterServerLoaded = loaded;
   GJVQSOLogFrame->setClusterServerLoaded(loaded);
   bandmapControlFrame->setClusterServerLoaded(loaded);
   clusterControlFrame->setClusterServerLoaded(loaded);
}

bool TSingleLogFrame::isClusterServerLoaded()
{
   return clusterServerLoaded;
}

void TSingleLogFrame::setClusterClientLoaded(bool loaded)
{
   clusterClientLoaded = loaded;
   GJVQSOLogFrame->setClusterClientLoaded(loaded);
}

bool TSingleLogFrame::isClusterClientLoaded()
{
   return clusterClientLoaded;
}

void TSingleLogFrame::on_setClusterTXSpotEnableState(QString state)
{
   bool txEnableState = false;
    if (state == SPOT_TX_ON)
    {
        txEnableState = true;
    }

    GJVQSOLogFrame->setClusterTXSpotEnableState(txEnableState);
}



void TSingleLogFrame::on_clusterServerState(QString state)
{
    clusterServerState = state;
    GJVQSOLogFrame->setClusterServerState(state);
    bandmapControlFrame->setClusterServerState(state);
    clusterControlFrame->setClusterServerState(state);
}

QString TSingleLogFrame::getClusterServerState()
{
    return clusterServerState;
}

//---------------------------------------------------------------------------
void TSingleLogFrame::checkConnections()
{
    // check on rig and rotator connections
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        if (FKHRigControlFrame)
            FKHRigControlFrame->checkConnection();
        if (FKHRotControlFrame)
            FKHRotControlFrame->checkConnection();
    }
}
//---------------------------------------------------------------------------

// RigControl

void TSingleLogFrame::on_SetMode(QString m)
{
    if (sCurMode != m)
    {
        if ( this == LogContainer->getCurrentLogFrame() )
        {
            sCurMode = m;
            FKHRigControlFrame->setMode(m);
            GJVQSOLogFrame->modeSentFromRig(m);
            bandmapControlFrame->setMode(m);
        }
    }
}

void TSingleLogFrame::on_SetFreq(Frequency f)
{
    traceMsg(QString("on_SetFreq: freq from radio = %1").arg(f.traceStr()));

    if (f.isClear())
    {
        traceMsg(QString("onSetFreq: freq is zero"));
        return;
    }

    if ( this == LogContainer->getCurrentLogFrame() )
    {
        bool stopKeyer = false;
        if (f != sCurFreq)
        {
            stopKeyer = true;
            trace(QString("Setting stop keyer f = %1 sCurFreq = %2").arg(f.traceStr()).arg(sCurFreq.traceStr()));
        }
        sCurFreq = f;
        FKHRigControlFrame->setFreq(f);
        runButtonsFrame->setFreq(f);
        GJVQSOLogFrame->setFreq(f);
        bandmapControlFrame->setFreq(f);
        if (pauseRigControlUpdates)    // pausing updates while contest is changing
        {

            if (f == FKHRigControlFrame->getSendFreq()) // waiting for correct freq on contest change
            {
                pauseRigControlUpdates = false;
                updateFreq(f);
            }

        }
        else
        {
            updateFreq(f);
        }


        if (stopKeyer)
        {
            trace("sendKeyerStop from TSingleLogFrame::on_setFreq");
            sendKeyerStop();    // if we have tuned, stop keyer
        }
    }

}

void TSingleLogFrame::updateFreq(Frequency f)
{
    sCurFreq = f;
    FKHRigControlFrame->setFreq(f);
    runButtonsFrame->setFreq(f);
    GJVQSOLogFrame->setFreq(f);
    bandmapControlFrame->setFreq(f);

    MinosLoggerEvents::sendRigFreqChanged(f, contest);
}


void TSingleLogFrame::setPauseRigControlUpdatesFlag(bool status)
{
    pauseRigControlUpdates = status;
}

void TSingleLogFrame::on_SetRitFreq(ShortFreq f)
{
    if (curRitFreq != f)
        if ( this == LogContainer->getCurrentLogFrame() )
        {
            curRitFreq = f;
            FKHRigControlFrame->setRitFreq(f);
        }
}

void TSingleLogFrame::on_SetRitRadioStatus(bool status)
{
    if (this == LogContainer->getCurrentLogFrame())
    {
        FKHRigControlFrame->setRitRadioStatus(status);
    }

}

void TSingleLogFrame::on_NoRadioSetFreq(Frequency f)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        GJVQSOLogFrame->setFreq(f);
    }
}

void TSingleLogFrame::on_NoRadioSetMode(QString m)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        GJVQSOLogFrame->modeSentFromRig(m);
    }
}

void TSingleLogFrame::on_SetVolume(int level)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRigControlFrame->setVolume(level);
    }
}

void TSingleLogFrame::on_RadioLoaded()
{
    FKHRigControlFrame->setRadioLoaded();
    GJVQSOLogFrame->setRadioLoaded();
}

bool TSingleLogFrame::isRadioLoaded()
{
   return FKHRigControlFrame->isRadioLoaded();
}

void TSingleLogFrame::on_SetRadioList()
{
    FKHRigControlFrame->setRadioListFromTslf();
}

void TSingleLogFrame::on_SetTransVertOffset(double offset, PubSubName psn)
{
    FKHRigControlFrame->setTransVertOffset( offset, psn);
}

void TSingleLogFrame::on_SetTransVertSwitch(int switchNum, PubSubName psn)
{
    FKHRigControlFrame->setTransVertSwitch(switchNum, psn);
}

void TSingleLogFrame::on_SetTransVertEnabled(bool status, PubSubName psn)
{
    FKHRigControlFrame->setTransVertEnabled(status, psn);
}


void TSingleLogFrame::on_SetTransVertStatus(bool status, PubSubName psn)
{
    FKHRigControlFrame->setTransVertStatus(status, psn);
}

void TSingleLogFrame::on_SetVolumeStatus(bool status, PubSubName psn)
{
    FKHRigControlFrame->setVolumeStatus( status, psn);
}

void TSingleLogFrame::on_SetRitEnableStatus(bool status, PubSubName psn)
{
    FKHRigControlFrame->setRitEnableStatus(status, psn);
}

void TSingleLogFrame::on_SetRitMaxKHzFreq(int maxRitFreq, PubSubName psn)
{
    FKHRigControlFrame->setRitMaxKHzFreq(maxRitFreq, psn);
}
void TSingleLogFrame::on_SetBandList(QString s,PubSubName psn)
{
    FKHRigControlFrame->setBandList(s, psn);
}


void TSingleLogFrame::on_SetRadioStatus(QString s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRigControlFrame->setRadioState(s);
        GJVQSOLogFrame->setRadioState(s);
    }
}

/*

void TSingleLogFrame::on_SetRadioVolumeState(bool s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRigControlFrame->setRadioVolumeState(s);
    }
}

void TSingleLogFrame::on_SetRitEnableState(bool s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRigControlFrame->setRitEnableState(s);
    }
}
*/
//---- Send to RigController




void TSingleLogFrame::sendRadioFreq(Frequency freq)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        trace("sendKeyerStop from TSingleLogFrame::sendRadioFreq");
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlFreq(this, freq);

    }
}

void TSingleLogFrame::sendRadioRitFreq(ShortFreq freq)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRigControlRitFreq(this, freq);
    }
}


void TSingleLogFrame::sendRadioRitStatus(bool status)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRigControlRitStatus(this, status);
    }
}

void TSingleLogFrame::sendRadioVolume(int level)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendRigControlVolumeLevel(this, level);
    }
}

void TSingleLogFrame::sendRadioMode(QString mode)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        trace("sendKeyerStop from TSingleLogFrame::sendRadioMode");
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlMode(this, mode);
    }
}

void TSingleLogFrame::sendSelectRadio(const QString &radName, const Frequency &freq, const QString &mode)
{

    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        if (ct && !ct->isReadOnly())
        {
            // make sure log frame has correct name for radio
            if (radName != GJVQSOLogFrame->getRadioName())
            {
               GJVQSOLogFrame->setRadioName(radName);
               //FKHRigControlFrame->setRadioName(radName, mode);
               //LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
            }

            LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
            QString uuid = ct->uuid;
            LogContainer->sendDM->changeRigSelectionTo(radName, freq, mode, ct->uuid);  // send message including mode if it has been appended.
            traceMsg(QString("changeRigSelectionTo radioName = %1, freq = %2, mode = %3, uuid = %4").arg(radName).arg(freq.traceStr()).arg(mode).arg(uuid));



            if (radName != ct->radioName.getValue().toString())
            {
                ct->radioName.setValue(radName);
                ct->commonSave(false);
            }
        }

    }
}



void TSingleLogFrame::invalidateCacheOnDisconnect()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        if (ct && !ct->isReadOnly())
        {
            LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
        }
    }
}


void TSingleLogFrame::sendSelectRotator(const QString &s)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        if (ct && !contest->isReadOnly())
        {
            // log frame doesn't record the antenna name

            if (s != ct->antennaName.getValue().toString())
            {
                ct->antennaName.setValue(s);
                ct->commonSave(false);

                //FKHRotControlFrame->setRotatorAntennaName(s);
            }
            LogContainer->sendDM->changeRotatorSelectionTo(ct->antennaName.getValue(), ct->uuid);
            LogContainer->sendDM->invalidateRotatorCache(ct->antennaName.getValue());
        }
    }

}

/*
void TSingleLogFrame::sendRadioPassBandState(int state)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendRigControlPassBandState(state);
}
*/

//---------------------------------------------------------------------------

// RotatorControl



void TSingleLogFrame::on_RotatorLoaded()
{
   rotatorLoaded = true;
   FKHRotControlFrame->setRotatorLoaded();
   GJVQSOLogFrame->setRotatorLoaded();
}

bool TSingleLogFrame::isRotatorLoaded()
{
   return rotatorLoaded;
}
void TSingleLogFrame::on_RotatorList()
{
    FKHRotControlFrame->setRotatorList();
}

void TSingleLogFrame::on_RotatorPresetList(QString s)
{
    rotPresets->setRotatorPresetList(s);
}


void TSingleLogFrame::on_RotatorStatus(QString s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRotControlFrame->setRotatorState(s);
    }
}

void TSingleLogFrame::on_RotatorBearing(QString s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRotControlFrame->setRotatorBearing(s);
        bandmapControlFrame->setRotatorBearing(s);
        GJVQSOLogFrame->setRotatorBearing(s);
    }
}

void TSingleLogFrame::on_rotatorConnected(bool connected)
{
    bandmapControlFrame->setRotatorConnected(connected);
}


void TSingleLogFrame::on_RotatorMaxAzimuth(int s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRotControlFrame->setRotatorMaxAzimuth(s);
    }
}

void TSingleLogFrame::on_RotatorMinAzimuth(int s)
{
    if ( this == LogContainer->getCurrentLogFrame() )
    {
        FKHRotControlFrame->setRotatorMinAzimuth(s);
    }
}

void TSingleLogFrame::on_SupportStopCommand(bool state)
{
    if (this == LogContainer->getCurrentLogFrame())
    {
        FKHRotControlFrame->setSupportStopCommandFlag(state);
    }
}

void TSingleLogFrame::on_cwCcwCmdEnable(bool s)
{
    if (this == LogContainer->getCurrentLogFrame())
    {
       FKHRotControlFrame->setCwCcwCmdEnable(s);
    }
}

void TSingleLogFrame::sendRotator(rpcConstants::RotateDirection direction, int angle )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendRotator(this, direction, angle);
}

void TSingleLogFrame::sendRotatorPreset(QString s )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        LogContainer->sendDM->sendRotatorPreset(s);
}

void TSingleLogFrame::presetTurn(QString b)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        FKHRotControlFrame->presetTurn(b);
}

//---------------------------------------------------------------------------

void TSingleLogFrame::traceMsg(QString msg)
{

     trace(QString("[TSingleLogFrame] %1").arg(msg));
}
