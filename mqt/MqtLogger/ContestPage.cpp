#include "base_pch.h"

#include "tlogcontainer.h"
#include "ScreenConfigScreen.h"
#include "minossplitter.h"
#include "tsinglelogframe.h"
#include "ContestPage.h"
#include "ui_ContestPage.h"

ContestPage::ContestPage(QWidget *parent, BaseContestLog *ct) :
    QFrame(parent),
    contest(ct),
    ui(new Ui::ContestPage),
  splittersChanged(false)
{
#ifdef Q_OS_ANDROID
    splitterHandleWidth = 20;
#else
    splitterHandleWidth = 6;
#endif

    ui->setupUi(this);

    singleLogFrameSplitter = new MinosSplitter();

    singleLogFrameSplitter->setObjectName(QStringLiteral("singleLogFrameSplitter"));
    singleLogFrameSplitter->setOrientation(Qt::Vertical);
    singleLogFrameSplitter->setChildrenCollapsible(false);

    connect(singleLogFrameSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(onSplitterMoved(int, int)));

    connect(&MinosLoggerEvents::mle, SIGNAL(SplittersChanged()), this, SLOT(onSplittersChanged()));
    connect(&MinosLoggerEvents::mle, SIGNAL(doSplitterChanges(BaseContestLog*)), this, SLOT(on_doSplitterChanges(BaseContestLog*)));

    connect(&MinosLoggerEvents::mle, SIGNAL(ContestShownChanged()), this, SLOT(on_ContestShownChanged()));
}

ContestPage::~ContestPage()
{
    delete ui;
}

BaseContestLog * ContestPage::getContest()
{
   return contest;
}

void ContestPage::buildScreen(TSingleLogFrame *tslfp, SCScreen &s, int auxInstance)
{

    tslf = tslfp;
    for (auto &r: s.baseElement->rows)
    {
        tslf->buildRow(r, auxInstance, singleLogFrameSplitter);
    }
    // set frame to Vertical Layout, insert LogFrameSplitter
    if (!verticalLayout)
    {
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->addWidget(singleLogFrameSplitter);
        setLayout(verticalLayout);
    }

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

    onSplitterMoved(-1, -1);
}

void ContestPage::clearScreen()
{
    QWidget *s = singleLogFrameSplitter->widget(0);
    while (s)
    {
        s->setParent(nullptr);
//        s->deleteLater();
        delete(s);
        s = singleLogFrameSplitter->widget(0);
    }
    rowSplitters.clear();
    if (tslf != this)
    {
        singleLogFrameSplitter = nullptr;
        delete(verticalLayout);
        verticalLayout = nullptr;
    }
}
void ContestPage::getSplitters()
{
    if (!singleLogFrameSplitter)
    {
        return;
    }
    QSettings settings;
    QByteArray state;

    QString name = QString("Splitters/%1/state/%2/%3").arg("singleLogFrameSplitter").arg(tslf->getCurScreenLayout()).arg(pageNo);
    state = settings.value(name).toByteArray();
    singleLogFrameSplitter->restoreState(state);

    // and reset some of the saved state

    singleLogFrameSplitter->setChildrenCollapsible(true);
    singleLogFrameSplitter->setHandleWidth(splitterHandleWidth);

    for(auto const &s: rowSplitters)
    {
        QByteArray sstate;
        QString name = QString("Splitters/%1/state/%2/%3").arg(s->objectName()).arg(tslf->getCurScreenLayout()).arg(pageNo);
        sstate = settings.value(name, sstate).toByteArray();
        s->restoreState(sstate);
        s->setHandleWidth(splitterHandleWidth);
        s->setChildrenCollapsible(true);
    }
}
void ContestPage::onSplittersChanged()
{
    splittersChanged = true;
}
void ContestPage::onSplitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = singleLogFrameSplitter->saveState();
    QSettings settings;
    QString name = QString("Splitters/%1/state/%2/%3").arg("singleLogFrameSplitter").arg(tslf->getCurScreenLayout()).arg(pageNo);
    settings.setValue(name, state);

    for(auto const &s: rowSplitters)
    {
        state = s->saveState();
        QString name = QString("Splitters/%1/state/%2/%3").arg(s->objectName()).arg(tslf->getCurScreenLayout()).arg(pageNo);
        settings.setValue(name, state);
        MinosLoggerEvents::SendSplittersChanged();
    }
}
void ContestPage::on_doSplitterChanges(BaseContestLog *b)
{
    if (b == contest)
    {
        getSplitters();
    }
}
void ContestPage::on_ContestShownChanged ()
{
    if (!ui)
        return;
    if ( tslf != LogContainer->getCurrentLogFrame() )
    {
        return;
    }

    if (splittersChanged)
    {
        MinosLoggerEvents::SendDoSplitterChanges(getContest());
        splittersChanged = false;
    }
}
