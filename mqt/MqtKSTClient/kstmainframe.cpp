#include <QScrollArea>

#include "MTrace.h"
#include "ScreenConfigManager.h"
#include "cutils.h"
#include "delayedaction.h"
#include "kstactivechatsframe.h"
#include "kstbuttonsframe.h"
#include "kstcallsframe.h"
#include "kstloginframe.h"
#include "kstmainwindow.h"
#include "kstmsgframe.h"
#include "kstplanesframe.h"
#include "kstsendmeepframe.h"
#include "ksttomeframe.h"
#include "minossplitter.h"

#include "kstmainframe.h"
#include "ui_kstmainframe.h"

bool KSTMainFrame::inApplyScreenLayout = false;

KSTMainFrame::KSTMainFrame(QWidget *parent)
    : KSTPageFrame(parent)
    , ui(new Ui::KSTMainFrame)
{
    ui->setupUi(this);
}

KSTMainFrame::~KSTMainFrame()
{
    delete ui;
}
void KSTMainFrame::traceMsg(QString msg)
{

    trace(QString("[KSTMainFrame] %1").arg(msg));
}
void KSTMainFrame::createScreenComponents()
{
    // create component frames, parentless

    traceMsg("createScreenComponents start");

    kstActiveChatsFrame = new KSTActiveChatsFrame(this);
    kstActiveChatsFrame->setObjectName(QStringLiteral("KSTActiveChatsFrame"));
    kstActiveChatsFrame->setFrameShape(QFrame::StyledPanel);
    kstActiveChatsFrame->setFrameShadow(QFrame::Raised);
    kstActiveChatsFrame->setVisible(false);

    kstButtonsFrame = new KSTButtonsFrame(this);
    kstButtonsFrame->setObjectName(QStringLiteral("KSTButtonsFrame"));
    kstButtonsFrame->setFrameShape(QFrame::StyledPanel);
    kstButtonsFrame->setFrameShadow(QFrame::Raised);
    kstButtonsFrame->setVisible(false);

    kstCallsFrame = new KSTCallsFrame(this);
    kstCallsFrame->setObjectName(QStringLiteral("KSTCallsFrame"));
    kstCallsFrame->setFrameShape(QFrame::StyledPanel);
    kstCallsFrame->setFrameShadow(QFrame::Raised);
    kstCallsFrame->setVisible(false);

    kstLoginFrame = new KSTLoginFrame(this);
    kstLoginFrame->setObjectName(QStringLiteral("KSTLoginFrame"));
    kstLoginFrame->setFrameShape(QFrame::StyledPanel);
    kstLoginFrame->setFrameShadow(QFrame::Raised);
    kstLoginFrame->setVisible(false);

    kstMsgFrame = new KSTMsgFrame(this);
    kstMsgFrame->setObjectName(QStringLiteral("KSTMsgFrame"));
    kstMsgFrame->setFrameShape(QFrame::StyledPanel);
    kstMsgFrame->setFrameShadow(QFrame::Raised);
    kstMsgFrame->setVisible(false);

    kstPlanesFrame = new KSTPlanesFrame(this);
    kstPlanesFrame->setObjectName(QStringLiteral("KSTPlanesFrame"));
    kstPlanesFrame->setFrameShape(QFrame::StyledPanel);
    kstPlanesFrame->setFrameShadow(QFrame::Raised);
    kstPlanesFrame->setVisible(false);

    kstSendMeepFrame = new KSTSendMeepFrame(this);
    kstSendMeepFrame->setObjectName(QStringLiteral("KSTSendMeepFrame"));
    kstSendMeepFrame->setFrameShape(QFrame::StyledPanel);
    kstSendMeepFrame->setFrameShadow(QFrame::Raised);
    kstSendMeepFrame->setVisible(false);

    kstTomeFrame = new KSTTomeFrame(this);
    kstTomeFrame->setObjectName(QStringLiteral("KSTTomeFrame"));
    kstTomeFrame->setFrameShape(QFrame::StyledPanel);
    kstTomeFrame->setFrameShadow(QFrame::Raised);
    kstTomeFrame->setVisible(false);

    traceMsg("createScreenComponents end");
}
void KSTMainFrame::clearScreenLayout(bool clearAllPages)
{
    // clear down the screen elements

    suppressSaveHeaders = true; // stop cutils saving headers

    traceMsg("clearScreenLayout starts ");


    kstActiveChatsFrame->setParent(this);
    kstActiveChatsFrame->hide();
    kstButtonsFrame->setParent(this);
    kstButtonsFrame->hide();
    kstCallsFrame->setParent(this);
    kstCallsFrame->hide();
    kstLoginFrame->setParent(this);
    kstLoginFrame->hide();

    kstMsgFrame->setParent(this);
    kstMsgFrame->hide();
    kstPlanesFrame->setParent(this);
    kstPlanesFrame->hide();
    kstSendMeepFrame->setParent(this);
    kstSendMeepFrame->hide();
    kstTomeFrame->setParent(this);
    kstTomeFrame->hide();

    if (clearAllPages)
    {
        for (auto cpc = pages.begin(); cpc != pages.end(); cpc++)
        {
            if (*cpc == nullptr)
            {
                continue;
            }
            (*cpc)->clearScreen();
            if ((*cpc) != this)
            {
                (*cpc)->deleteLater();
            }
        }
        pages.clear();
    }
    clearScreen();
    suppressSaveHeaders = false;
    traceMsg("clearScreenLayout complete");
}
void KSTMainFrame::applyScreenLayout()
{
    inApplyScreenLayout = true;

    traceMsg("applyScreenLayout start");

    clearScreenLayout(true);
    buildScreenLayout();
    mainWindow->setContentsMargins(0, 0, 0, 0);

    delayedAction(this,  [=](){
        inApplyScreenLayout = false;
    }
    );
}

QString KSTMainFrame::getCurScreenLayout() const
{
    return curScreenLayout;
}

void KSTMainFrame::setCurScreenLayout(const QString &value)
{
    trace(QString("setCurScreenLayout %1").arg(value));
    curScreenLayout = value;
}

void KSTMainFrame::on_FontChanged()
{
    kstActiveChatsFrame->on_FontChanged();
    kstButtonsFrame->on_FontChanged();
    kstCallsFrame->on_FontChanged();
    kstLoginFrame->on_FontChanged();
    kstMsgFrame->on_FontChanged();
    kstPlanesFrame->on_FontChanged();
    kstSendMeepFrame->on_FontChanged();
    kstTomeFrame->on_FontChanged();
}
void KSTMainFrame::buildRow(KSTPageFrame *cp, SCRow &scrow, MinosSplitter *splitterParent)
{
    // This builds the dependant ContestPage (including the one we derive from)
    if (scrow.elements.count())
    {
        // insert horizontal splitter in splitterParent
        MinosSplitter *hs = new MinosSplitter();
        hs->setObjectName("row" + QString::number(cp->rowSplitters.size()) + "splitter");
        hs->setOrientation(Qt::Horizontal);
        hs->setChildrenCollapsible(false);

        // we want this in the contest page...
        cp->rowSplitters.push_back(hs);

        for (auto &scele: scrow.elements)
        {
            //SCElement scele = scrow.elements[srele];
            SCType type = scele.type;
            if (type == sctNone)
                continue;

            QScrollArea *elementScrollArea = nullptr;
            if (type == sctkActiveChats ||
                type == sctkLogins ||
                type == sctkSendMeep ||
                type == sctkButtons
                )
            {
                elementScrollArea = new QScrollArea();
                elementScrollArea->setWidgetResizable(true);
                elementScrollArea->setFocusPolicy(Qt::NoFocus);
                elementScrollArea->setFrameStyle(QStyleOptionFrame::None);
                elementScrollArea->setFrameShadow(QFrame::Plain);

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

            case sctkActiveChats:
                elementScrollArea->setWidget(kstActiveChatsFrame);
                break;

            case sctkCallList:
                hs->addWidget(kstCallsFrame);
                kstCallsFrame->setVisible(true);
                break;

            case sctkAirScout:
                hs->addWidget(kstPlanesFrame);
                kstPlanesFrame->setVisible(true);
                break;

            case sctkMessageList:
                hs->addWidget(kstMsgFrame);
                kstMsgFrame->setVisible(true);
                break;

            case sctkMeepList:
                hs->addWidget(kstTomeFrame);
                kstTomeFrame->setVisible(true);
                break;

            case sctkLogins:
                elementScrollArea->setWidget(kstLoginFrame);
                break;

            case sctkSendMeep:
                elementScrollArea->setWidget(kstSendMeepFrame);
                break;

            case sctkButtons:
                elementScrollArea->setWidget(kstButtonsFrame);
                break;

            case sctSplit:
            {
                MinosSplitter *vs = new MinosSplitter();
                vs->setObjectName("splitRow" + QString::number(cp->rowSplitters.size()) + "splitter");
                vs->setOrientation(Qt::Vertical);
                vs->setChildrenCollapsible(false);
                cp->rowSplitters.push_back(vs);

                for (auto &srow: scele.rows)
                {
                    buildRow(cp, srow, vs);
                }

                hs->addWidget(vs);
                break;
            }
            default:
                break;
         }
        }
        splitterParent->addWidget(hs);
    }

}
void KSTMainFrame::buildScreen(SCScreen &s, int t)
{
    // we need to add this contest page to the relevant contestPageControl
    // as a new tab

    // How do we make sure that ALL contests are in ALL page controls, even when
    // they have no such screen?

    KSTPageFrame *cp = this;
    if (t > 0)
    {
        cp = new KSTPageFrame(nullptr);
        QString n = QString("kstpage%1").arg(t);
        cp->setObjectName(n);
        pages.push_back(cp);
        cp->setWindowFlags(/*Qt::Tool |*/ Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint);
        cp->setAttribute(Qt::WA_ShowWithoutActivating);
        cp->show();
    }
    cp->pageNo = t;
    cp->buildScreen(this, s);

}
void KSTMainFrame::buildScreenLayout()
{
    suppressSaveHeaders = true; // stop cutils saving headers

    ScreenConfigFile &scf = ScreenConfigFile::getScreenConfigFile(this);

    if (curScreenLayout.isEmpty() || !scf.configs.contains(curScreenLayout))
    {
        curScreenLayout = defaultLayoutName();
    }
    traceMsg("buildScreenLayout to layout " + curScreenLayout);
    setCurScreenLayout(curScreenLayout);

    SC sc = scf.configs[curScreenLayout];

    // build the pages
    int t = 0;
    for (auto &s: sc.baseElement->screens)
    {
        buildScreen(s, t++);
    }

    suppressSaveHeaders = false;
}
