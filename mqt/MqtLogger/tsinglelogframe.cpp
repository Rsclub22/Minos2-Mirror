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

#include "tsinglelogframe.h"
#include "ui_tsinglelogframe.h"

TSingleLogFrame::TSingleLogFrame(QWidget *parent, BaseContestLog * contest) :
    QFrame(parent),
    ui(new Ui::TSingleLogFrame),
    splittersChanged(false),
    bandMapLoaded(false),
    rotatorLoaded(false),
    keyerLoaded(false),
    radioLoaded(false),
    contest(contest),
    lastStanzaCount( 0 )


{
#ifdef Q_OS_ANDROID
    splitterHandleWidth = 20;
#else
    splitterHandleWidth = 6;
#endif

    ui->setupUi(this);

    createScreenComponents();

    buildScreenLayout();

    OtherMatchTreeFW = new FocusWatcher(otherMatchFrame->getTreeView());
    connect(OtherMatchTreeFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(onOtherMatchTreeFocused(QObject *, bool, QFocusEvent *)));
    ArchiveMatchTreeFW = new FocusWatcher(archiveMatchFrame->getTreeView());
    connect(ArchiveMatchTreeFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(onArchiveTreeFocused(QObject *, bool, QFocusEvent *)));

    restoreColumns();

    connect(&MinosLoggerEvents::mle, SIGNAL(ContestPageChanged()), this, SLOT(on_ContestPageChanged()));

    //connect(&MinosLoggerEvents::mle, SIGNAL(BandMapPressed()), this, SLOT(on_BandMapPressed()));

    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(NextContactDetailsTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(PublishTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(HideTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(MakeEntry(BaseContestLog*)), this, SLOT(on_MakeEntry(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)), this, SLOT(on_AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)));
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterLogContact(BaseContestLog *)), this, SLOT(on_AfterLogContact(BaseContestLog *)));
    connect(&MinosLoggerEvents::mle, SIGNAL(setMemory(BaseContestLog *, QString, QString)), this, SLOT(on_SetMemory(BaseContestLog *, QString, QString)));
    // from cluster frame or bandmap frame
    connect(&MinosLoggerEvents::mle, SIGNAL(DxSpotToMemory(BaseContestLog *, memoryData::memData)), this, SLOT(on_dxSpotToMemory(BaseContestLog *, memoryData::memData)));
    connect(&MinosLoggerEvents::mle, SIGNAL(MatchStarting(BaseContestLog*)), this, SLOT(on_MatchStarting(BaseContestLog*)));

    connect(&MinosLoggerEvents::mle, SIGNAL(ColumnsChanged()), this, SLOT(onColumnsChanged()));
    connect(&MinosLoggerEvents::mle, SIGNAL(SplittersChanged()), this, SLOT(onSplittersChanged()));
    connect(&MinosLoggerEvents::mle, SIGNAL(NextUnfilled(BaseContestLog*)), this, SLOT(on_NextUnfilled(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(GoToSerial(BaseContestLog*)), this, SLOT(on_GoToSerial(BaseContestLog*)));

    connect(&MinosLoggerEvents::mle, SIGNAL(XferPressed(BaseContestLog *, QString)), this, SLOT(on_XferPressed(BaseContestLog *, QString)));
    connect(thisMatchFrame, SIGNAL(editContact(QSharedPointer<BaseContact>)), this, SLOT(EditContact(QSharedPointer<BaseContact>)));

    connect(&MinosLoggerEvents::mle, SIGNAL(XferEnabled(bool, BaseContestLog *, QString)), GJVQSOLogFrame, SLOT(setXferEnabled(bool, BaseContestLog *, QString)));
    connect(&MinosLoggerEvents::mle, SIGNAL(MatchTreeSelected(MatchType , BaseContestLog *, QString, QItemSelection)),
            this, SLOT(MatchTreeSelected(MatchType, BaseContestLog *, QString, QItemSelection)));

    connect(&MinosLoggerEvents::mle, SIGNAL(doColumnChanges(BaseContestLog*)), this, SLOT(on_doColumnChanges(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(doSplitterChanges(BaseContestLog*)), this, SLOT(on_doSplitterChanges(BaseContestLog*)));


    // RigControl Updates
    // From rig controller
    connect(LogContainer->sendDM, SIGNAL(setRadioLoaded()), this, SLOT(on_RadioLoaded()));
    connect(LogContainer->sendDM, SIGNAL(setRadioList()), this, SLOT(on_SetRadioList()));

    // To rig controller

    connect(FKHRigControlFrame, SIGNAL(radioDisconnected()), this, SLOT(invalidateCacheOnDisconnect()));
    connect(FKHRigControlFrame, SIGNAL(selectRadio(QString, QString)), this, SLOT(sendSelectRadio(QString, QString)));

    connect(FKHRigControlFrame, SIGNAL(sendFreqControl(QString)), this, SLOT(sendRadioFreq(QString)));
    connect(GJVQSOLogFrame, SIGNAL(sendFreqControl(QString)), this, SLOT(sendRadioFreq(QString)));
    connect(FKHRigControlFrame, SIGNAL(sendRitFreq(int)), this, SLOT(sendRadioRitFreq(int)));
    connect(FKHRigControlFrame, SIGNAL(sendVolumeToRadio(int)), this, SLOT(sendRadioVolume(int)));
    connect(FKHRigControlFrame, SIGNAL(ritStatus(bool)), this, SLOT(sendRadioRitStatus(bool)));
    connect(FKHRigControlFrame, SIGNAL(sendModeToControl(QString)), this, SLOT(sendRadioMode(QString)));
    connect(GJVQSOLogFrame, SIGNAL(sendModeControl(QString)), this , SLOT(sendRadioMode(QString)));

    connect(FKHRigControlFrame, SIGNAL(sendRunOnFlag(QString, bool)), this, SLOT(sendRunOnFlag(QString, bool)));
    connect(FKHRigControlFrame, SIGNAL(sendRunOffFreqFlag(QString, bool)), this, SLOT(sendRunOffFreqFlag(QString, bool)));


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


    // to cluster server
    connect(GJVQSOLogFrame, SIGNAL(sendSpotToClusterServer(QString, QString, QString)), this, SLOT(on_SendSpotToClusterServer(QString, QString, QString)));

    // from cluster server
    connect(LogContainer->sendDM, SIGNAL(setClusterServerLoaded()),this, SLOT(on_clusterServerLoaded()));
    connect(LogContainer->sendDM, SIGNAL(setClusterState(QString)), this, SLOT(on_clusterServerState(QString)));
    connect(LogContainer->sendDM, SIGNAL(setClusterTXSpotEnableState(QString)), this, SLOT(on_setClusterTXSpotEnableState(QString)));


    // to bandmap
    connect(GJVQSOLogFrame, SIGNAL(bandmapMarkFreq(QString, QString, QString, QString)),
            this, SLOT(on_BandmapMarkFreq(QString, QString, QString, QString)));
    connect(GJVQSOLogFrame, SIGNAL(bandmapSaveFreq(QString, QString, QString, QString)),
            this, SLOT(on_BandmapSaveFreq(QString, QString, QString, QString)));
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

void TSingleLogFrame::on_FontChanged()
{
    applyScreenLayout();
}
TSingleLogFrame::~TSingleLogFrame()
{
    delete ui;

    ui = nullptr;
    contest = nullptr;

    delete clusterControlFrame;
    delete wsjtxFrame;
}
void TSingleLogFrame::createScreenComponents()
{
    // create component frames, parentless
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    trace("TSingleLogFrame::createScreenComponents for " + ct->name.getValue() + " uuid " + ct->uuid);

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
    GJVQSOLogFrame->setXferEnabled(false, contest, "Log");
    GJVQSOLogFrame->initialise( contest );

    FKHRigControlFrame = new RigControlFrame(this);
    FKHRigControlFrame->setObjectName(QStringLiteral("FKHRigControlFrame"));
    FKHRigControlFrame->setFrameShape(QFrame::StyledPanel);
    FKHRigControlFrame->setFrameShadow(QFrame::Raised);

    FKHRigControlFrame->setVisible(false);
    FKHRigControlFrame->setContest(contest);

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

    // set frame to Vertical Layout, insert LogFrameSplitter
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(0);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    singleLogFrameSplitter = new MinosSplitter(this);

    singleLogFrameSplitter->setObjectName(QStringLiteral("singleLogFrameSplitter"));
    singleLogFrameSplitter->setOrientation(Qt::Vertical);
    singleLogFrameSplitter->setChildrenCollapsible(false);

    connect(singleLogFrameSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(onSplitterMoved(int, int)));
}
void TSingleLogFrame::clearSplitter(MinosSplitter *s)
{
    while(s && s->count())
    {
        QWidget *w = s->widget(0);
        MinosSplitter *ws = dynamic_cast<MinosSplitter *>(w);

        if (ws)
        {
            clearSplitter(ws);
            ws->setParent(nullptr);
            ws->deleteLater();
        }
        else
        {
            // normal components - keep them built, but out of the way (not shown)
            // A lot ofthe code relies on them existing
            // aux components - delete them, and recreate them as necessary

            // but get rid of any scroll area "wrappers".

            w->hide();
            w->setParent(this);

            QScrollArea *qsa = dynamic_cast<QScrollArea *>(w);
            if (qsa)
            {
                QWidget *tw = qsa->takeWidget();
                qsa->deleteLater();
                if (tw)
                {
                    tw->hide();
                    tw->setParent(this);
                    StackedInfoFrame *aux = dynamic_cast<StackedInfoFrame *>(tw);
                    if (aux)
                    {
                        aux->setContest(nullptr);
                        aux->deleteLater();
                    }
                }
            }
        }
    }

}
void TSingleLogFrame::clearScreenLayout()
{
    // clear down the screen elements, but don't delete them (except for the aux frames) - they will be used to rebuild the screen
    // BUT on contest creation, the contest address may change, so clear the contest
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    QString msg = ct->name.getValue() + " uuid " + ct->uuid;
    trace("TSingleLogFrame::clearScreenLayout starts for " + msg);

    FKHRigControlFrame->setContest(nullptr);
    FKHRotControlFrame->setContest(nullptr);
    rotPresets->setContest(nullptr);
    thisMatchFrame->setContest(nullptr);
    otherMatchFrame->setContest(nullptr);
    archiveMatchFrame->setContest(nullptr);
    clusterControlFrame->setContest(nullptr);
    setClusterClientLoaded(false);
    bandmapControlFrame->setContest(nullptr);
    setBandmapLoaded(false);
    wsjtxFrame->setContest(nullptr);

    trace("TSingleLogFrame::clearScreenLayout start clearance for " + msg);

    while (singleLogFrameSplitter->count())
    {
        MinosSplitter *s = dynamic_cast<MinosSplitter *>(singleLogFrameSplitter->widget(0));
        clearSplitter(s);
        s->setParent(nullptr);
        s->deleteLater();
    }
    rowSplitters.clear();
    update();
    trace("TSingleLogFrame::clearScreenLayout complete for " + msg);
}
void TSingleLogFrame::applyScreenLayout()
{
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    trace("TSingleLogFrame::applyScreenLayout for " + ct->name.getValue() + " uuid " + ct->uuid);
    hide();
    QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    clearScreenLayout();
    buildScreenLayout();
    show();
    QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    onSplitterMoved(-1, -1);
}

QString TSingleLogFrame::getCurScreenLayout() const
{
    return curScreenLayout;
}

void TSingleLogFrame::setCurScreenLayout(const QString &value)
{
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
    ct->screenLayout.setValue(value);
    ct->commonSave(false);
}
void TSingleLogFrame::buildRow(SCRow &scrow, int &auxInstance, MinosSplitter *splitterParent)
{
    if (scrow.elements.count())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

        // insert horizontal splitter in splitterParent
        MinosSplitter *hs = new MinosSplitter();
        hs->setObjectName("row" + QString::number(rowSplitters.size()) + "splitter");
        hs->setOrientation(Qt::Horizontal);
        hs->setChildrenCollapsible(false);
        rowSplitters.push_back(hs);

        for (int srele = 0; srele < scrow.elements.count(); srele++)
        {
            SCElement scele = scrow.elements[srele];
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
                    FKHRigControlFrame->setContest(ct);
                    break;
                }
                case sctRotControl:
                {
                    elementScrollArea->setWidget(FKHRotControlFrame);
                    FKHRotControlFrame->setContest(ct);
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

                    for (int srow = 0; srow < scele.rows.count(); srow++)
                    {
                        buildRow(scele.rows[srow], auxInstance, vs);
                    }

                    hs->addWidget(vs);
                    break;
                }
            }
        }
        splitterParent->addWidget(hs);
    }

}
void TSingleLogFrame::buildScreenLayout()
{

    ScreenConfigFile scf;
    scf.loadFile(false, this);

    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
    QString curConfigName = ct->screenLayout.getValue();
    trace("TSingleLogFrame::buildScreenLayout for " + ct->name.getValue() + " uuid " + ct->uuid + " to layout " + curConfigName);
    if (curConfigName.isEmpty() || !scf.configs.contains(curConfigName))
    {
        curConfigName = defaultLayoutName();
        if ( !scf.configs.contains(curConfigName))
        {
            //we need to get the built in default
            scf.loadFile(true, this);
        }
    }
    curScreenLayout = curConfigName;
    SC sc = scf.configs[curConfigName];

    int auxInstance = 0;
    for (int j = 0; j < sc.baseElement->rows.count(); j++)
    {

        buildRow(sc.baseElement->rows[j], auxInstance, singleLogFrameSplitter);

    }
    // ALWAYS link the wsjt frame to the contest; then we can log
    // even without showing it
    wsjtxFrame->setContest(ct);
    verticalLayout->addWidget(singleLogFrameSplitter);

    getSplitters();

    // and force matters that may have been saved

    for (int i = 0; i < singleLogFrameSplitter->count(); i++)
    {
        singleLogFrameSplitter->setStretchFactor(i, 0);
    }
    for (int i = 0; i < rowSplitters.count(); i++)
    {
        for (int j = 0; j < rowSplitters[i]->count(); j++)
        {
            rowSplitters[i]->setStretchFactor(j, 0);
        }
        connect(rowSplitters[i], SIGNAL(splitterMoved(int, int)), this, SLOT(onSplitterMoved(int, int)));
    }

}


void TSingleLogFrame::keyPressEvent( QKeyEvent* event )
{

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
BaseContestLog * TSingleLogFrame::getContest()
{
   return contest;
}

void TSingleLogFrame::closeContest()
{
    if ( TContestApp::getContestApp() )
    {
       FKHRigControlFrame->closeContest();          // this disconnects rig on last closing contest
       FKHRotControlFrame->closeContest();
       RPCPubSub::publish( rpcConstants::monitorLogCategory, contest->publishedName, QString::number( 0 ), psRevoked );

       clearScreenLayout();
       TContestApp::getContestApp() ->closeFile( contest );
       GJVQSOLogFrame->closeContest();
       qsoModel.initialise(nullptr);

       contest = nullptr;
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
    trace("on_ContestPageChanged to " + ct->name.getValue() + " uuid " + ct->uuid);
    TContestApp::getContestApp() ->setCurrentContest( ct );

    if ( columnsChanged )
    {
        MinosLoggerEvents::SendDoColumnChanges(ct);             // this does a restorePartial in showQSOs
        columnsChanged = false;
    }

    if (splittersChanged)
    {
        MinosLoggerEvents::SendDoSplitterChanges(ct);
        splittersChanged = false;
    }

    refreshMults();

    GJVQSOLogFrame->selectField(nullptr);
    GJVQSOLogFrame->logTabChanged();

    MinosLoggerEvents::SendShowOperators();

    LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
    LogContainer->sendDM->invalidateRotatorCache(ct->antennaName.getValue());

    // save current freq as notifyRigChange writes incorrect contest freq to this frame
    // with a contest change
    sSavedCurFreq = sCurFreq;
    trace(QString("on_ContestPageChanged:: save current freq = %1, for frame = %2").arg(sSavedCurFreq).arg(ct->name.getValue() + " uuid " + ct->uuid));
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
void TSingleLogFrame::on_doSplitterChanges(BaseContestLog *b)
{
    if (b == contest)
    {
        getSplitters();
    }
}

void TSingleLogFrame::NextContactDetailsTimerTimer( )
{
    if ( contest )
    {
        QString cb = contest->band.getValue().trimmed();
        BandList &blist = BandList::getBandList();
        BandInfo bi;
        bool bandOK = blist.findBand(cb, bi);
        if (bandOK)
        {
            cb = bi.uk;
        }

        //we want to put a line across, and colour the bands - need a map of band->colour
        // ideally we want it configurable...

        CurrentBandLabel->setText( HtmlFontColour(bi.bandColour) + "<b><center><nobr><p><big><h1>" + cb);

        if ( contest->isReadOnly() )
        {
            NextContactDetailsLabel->setText( "<b><center><nobr><p><big><h1>"
                                                  + contest->mycall.fullCall.getValue() + "<br>"
                                                  + contest->myloc.loc.getValue() + "<br>"
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
                                                  + contest->mycall.fullCall.getValue() + "<br>"
                                                  + snBuff + "<br>"
                                                  + contest->myloc.loc.getValue()
                                                  + locBuff);
        }
    }
}
void TSingleLogFrame::PublishTimerTimer(  )
{
   LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>( contest );
   if ( ct && ct->isMinosFile() && !ct->isUnwriteable() && !ct->isProtected())
   {
      int stanzaCount = contest->getCtStanzaCount();
      if ( lastStanzaCount != stanzaCount )
      {
         // publish this contest details - what to use?
         // category LoggerContestLog
         // name filename(?)
         // value stanzaCount
          QString name = contest->name.getValue();
          QString band = contest->band.getValue();

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

    bool controlsLoaded = isBandMapLoaded() || isRadioLoaded() || isRotatorLoaded();

    if (controlsLoaded && !contest->isReadOnly())
    {
        if (FKHRigControlFrame->parent() != this)
            FKHRigControlFrame->setVisible(isRadioLoaded());
        if (FKHRotControlFrame->parent() != this)
            FKHRotControlFrame->setVisible(isRotatorLoaded());
        if (rotPresets->parent() != this)
            rotPresets->setVisible(isRotatorLoaded());
    }
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


void TSingleLogFrame::on_SendSpotToClusterServer(QString freq, QString callsign, QString loc)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LogContainer->sendDM->sendSpotToClusterServer(freq, callsign, loc);
    }
}

void TSingleLogFrame::transferDetails(memoryData::memData &m )
{
    if ( !contest  )
    {
       return ;
    }

    GJVQSOLogFrame->transferDetails( m.callsign, m.locator, m.fromBandmapOrMemory );
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

   qdlg.exec();

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
        QSOTable->scrollToBottom();
        int row = QSOTable->model()->rowCount() - 1;
        if (row >= 0)
        {
            QModelIndex index = QSOTable->model()->index( row, 0 );
            QSOTable->setCurrentIndex(index);
        }
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
void TSingleLogFrame::getSplitters()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("Splitters/singleLogFrameSplitter/state/" + curScreenLayout).toByteArray();
    singleLogFrameSplitter->restoreState(state);

    // and reset some of the saved state

    singleLogFrameSplitter->setChildrenCollapsible(false);
    singleLogFrameSplitter->setHandleWidth(splitterHandleWidth);

    foreach(MinosSplitter *s, rowSplitters)
    {
        QByteArray sstate;
        QString name = s->objectName();
        sstate = settings.value("Splitters/" + name + "/state/" + curScreenLayout, sstate).toByteArray();
        s->restoreState(sstate);
        s->setHandleWidth(splitterHandleWidth);
        s->setChildrenCollapsible(false);
    }
}
void TSingleLogFrame::onSplittersChanged()
{
    splittersChanged = true;
}
void TSingleLogFrame::onSplitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = singleLogFrameSplitter->saveState();
    QSettings settings;
    settings.setValue("Splitters/singleLogFrameSplitter/state/" + curScreenLayout, state);

    foreach(MinosSplitter *s, rowSplitters)
    {
        state = s->saveState();
        QString name = s->objectName();
        settings.setValue("Splitters/" + name + "/state/" + curScreenLayout, state);
        MinosLoggerEvents::SendSplittersChanged();
    }
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
    for ( LogIterator i = contest->ctList.begin(); i != contest->ctList.end(); i++ )
    {
        bool ok;
        int s = i->wt->serials.getValue().toInt(&ok );
       if ( ok && serial == s )
       {
          cfu = i->wt;
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


void TSingleLogFrame::on_BandmapMarkFreq(QString cs, QString freq, QString loc, QString brg)
{
    bandmapControlFrame->setBandmapMarkFreq(cs, freq, loc, brg);
}


void TSingleLogFrame::on_BandmapSaveFreq(QString cs, QString freq, QString loc, QString brg)
{
    bandmapControlFrame->setBandmapSaveFreq(cs, freq, loc, brg);
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

void TSingleLogFrame::sendRunOnFlag(QString runFreq, bool runModeOn)
{
    GJVQSOLogFrame->setRunOnFlag(runModeOn);
    bandmapControlFrame->setRunOnFlag(runFreq, runModeOn);
}

void TSingleLogFrame::sendRunOffFreqFlag(QString runFreq, bool offRunFreq)
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
        }
    }
}

void TSingleLogFrame::on_SetFreq(QString f)
{
    trace(QString("Freq from radio = %1").arg(f));
    if ( this == LogContainer->getCurrentLogFrame() )
    {

        sCurFreq = f;
        FKHRigControlFrame->setFreq(f);
        GJVQSOLogFrame->setFreq(f);
        //if (isBandMapLoaded())
        //{
           bandmapControlFrame->setFreq(f);
       // }

        MinosLoggerEvents::sendRigFreqChanged(f, contest);
    }

}

void TSingleLogFrame::on_SetRitFreq(QString f)
{
    if (sCurRitFreq != f)
        if ( this == LogContainer->getCurrentLogFrame() )
        {
            sCurRitFreq = f;
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

void TSingleLogFrame::on_NoRadioSetFreq(QString f)
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
    FKHRigControlFrame->setRadioList();
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




void TSingleLogFrame::sendRadioFreq(QString freq)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlFreq(this, freq);
    }
}

void TSingleLogFrame::sendRadioRitFreq(int freq)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlRitFreq(this, freq);
    }
}


void TSingleLogFrame::sendRadioRitStatus(bool status)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlRitStatus(this, status);
    }
}

void TSingleLogFrame::sendRadioVolume(int level)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlVolumeLevel(this, level);
    }
}

void TSingleLogFrame::sendRadioMode(QString mode)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        sendKeyerStop();    // don't keep calling while tuning!
        LogContainer->sendDM->sendRigControlMode(this, mode);
    }
}

void TSingleLogFrame::sendSelectRadio(const QString &radName, const QString &mode)
{

    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        if (ct && !ct->isProtected())
        {
            // make sure log frame has correct name for radio
            if (radName != GJVQSOLogFrame->getRadioName())
            {
               GJVQSOLogFrame->setRadioName(radName);
               //FKHRigControlFrame->setRadioName(radName, mode);
               //LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
            }

            LogContainer->sendDM->invalidateRigCache(ct->radioName.getValue());
            LogContainer->sendDM->changeRigSelectionTo(radName, mode, ct->uuid);  // send message including mode if it has been appended.




            if (radName != ct->radioName.getValue().toString())
            {
                ct->radioName.setValue(radName);
                ct->commonSave(false);
            }
        }

    }
}




//void TSingleLogFrame::sendIgnoreRunChkBoxState(int num, bool checked)
//{
//    GJVQSOLogFrame->setIgnoreRunChkBoxState(num, checked);
//}

void TSingleLogFrame::invalidateCacheOnDisconnect()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
        if (ct && !ct->isProtected())
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
        if (ct && !contest->isProtected())
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

