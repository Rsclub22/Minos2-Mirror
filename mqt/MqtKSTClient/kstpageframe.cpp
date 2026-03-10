#include <QVBoxLayout>
#include <QSettings>

#include "WindowsAppId.h"
#include "SecondInstall.h"

#include "QtUtils.h"
#include "kstmainwindow.h"
#include "minossplitter.h"
#include "regsettings.h"

#include "kstpageframe.h"
#include "ui_kstpageframe.h"

KSTPageFrame::KSTPageFrame(QWidget *parent, int pno)
    : QFrame(parent)
    , ui(new Ui::KSTPageFrame)
    , pageNo(pno)
{
    ui->setupUi(this);
#ifdef Q_OS_ANDROID
    splitterHandleWidth = 20;
#else
    splitterHandleWidth = 6;
#endif

    ui->setupUi(this);

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect(this, &KSTPageFrame::customContextMenuRequested, this, &KSTPageFrame::onCustomContextMenuRequested);

    if (pageNo > 0)
    {
        // we seem to have to do the geometry before the appId stuff,
        // or the geometry stuff doesn't work - don't know why!

        RegSettings settings;
        QByteArray geometry = settings.getSettings().value(QString("screen%1/geometry").arg(pageNo)).toByteArray();
        if (geometry.size() > 0)
        {
            restoreGeometry(geometry);
        }
    }
    bool sep = mainWindow->splitIcons;

    int subinst = 0;
    if (sep)
        subinst = pageNo;

    // the string shoud be formed from "CompanyName.ProductName.SubProduct.VersionInformation"
    // cf https://docs.microsoft.com/en-us/windows/win32/shell/appids

    setWinAppId(this, SecondInstall::getOrgName() + QString(".MqtKstClient.SubScreen%1").arg(subinst) );
}

KSTPageFrame::~KSTPageFrame()
{
    delete ui;
}
void KSTPageFrame::onCustomContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = mapToGlobal( pos );

    mainWindow->kstPopup.popup( globalPos );
}

void KSTPageFrame::buildScreen(SCScreen &s)
{
    kstPageSplitter = new MinosSplitter();

    kstPageSplitter->setObjectName(QStringLiteral("singleLogFrameSplitter"));
    kstPageSplitter->setOrientation(Qt::Vertical);
    kstPageSplitter->setChildrenCollapsible(false);

    pageName = s.name;
    for (auto &r: s.baseElement->rows)
    {
        mainWindow->buildRow(this, r, kstPageSplitter);
    }
    // set frame to Vertical Layout, insert LogFrameSplitter
    if (!verticalLayout)
    {
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->addWidget(kstPageSplitter);
        setLayout(verticalLayout);
    }

    getSplitters();

    // and force matters that may have been saved

    for (int i = 0; i < kstPageSplitter->count(); i++)
    {
        kstPageSplitter->setStretchFactor(i, 0);
    }
    for (int i = 0; i < rowSplitters.count(); i++)
    {
        for (int j = 0; j < rowSplitters[i]->count(); j++)
        {
            rowSplitters[i]->setStretchFactor(j, 0);

            for(int n = 0; n < rowSplitters[i]->count(); n++)
            {
                QWidget *w = rowSplitters[i]->widget(n);
                if (w)
                {
                    w->setMinimumWidth(20);
                    w->setMinimumHeight(20);
                }
            }
        }
        connect(rowSplitters[i], &MinosSplitter::splitterMoved, this, &KSTPageFrame::onSplitterMoved);
    }

    onSplitterMoved(-1, -1);
}

void KSTPageFrame::clearScreen()
{
    if (kstPageSplitter)
    {
        QWidget *s = kstPageSplitter->widget(0);
        while (s)
        {
            s->setParent(nullptr);
            delete(s);
            s = kstPageSplitter->widget(0);
        }
        rowSplitters.clear();

        kstPageSplitter = nullptr;
        delete(verticalLayout);
        verticalLayout = nullptr;
    }
}

bool KSTPageFrame::hasElements()
{
    return kstPageSplitter->count() > 0;
}
void KSTPageFrame::getSplitters()
{
    if (!kstPageSplitter)
    {
        return;
    }
    RegSettings settings;
    QByteArray state;

    QString name = QString("Splitters/%1/state/%2/%3").arg("singleLogFrameSplitter", mainWindow->getCurScreenLayout()).arg(pageNo);
    state = settings.getSettings().value(name).toByteArray();
    kstPageSplitter->restoreState(state);

    // and reset some of the saved state

    kstPageSplitter->setChildrenCollapsible(true);
    kstPageSplitter->setHandleWidth(splitterHandleWidth);

    for(auto const &s: QASCONST(rowSplitters))
    {
        QByteArray sstate;
        QString name = QString("Splitters/%1/state/%2/%3").arg(s->objectName(), mainWindow->getCurScreenLayout()).arg(pageNo);
        sstate = settings.getSettings().value(name, sstate).toByteArray();
        s->restoreState(sstate);
        s->setHandleWidth(splitterHandleWidth);
        s->setChildrenCollapsible(true);
    }
}
void KSTPageFrame::onSplittersChanged()
{
    splittersChanged = true;
}
void KSTPageFrame::onSplitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = kstPageSplitter->saveState();
    RegSettings settings;
    QString name = QString("Splitters/%1/state/%2/%3").arg("singleLogFrameSplitter", mainWindow->getCurScreenLayout()).arg(pageNo);
    settings.getSettings().setValue(name, state);

    for(auto const &s: QASCONST(rowSplitters))
    {
        state = s->saveState();
        QString name = QString("Splitters/%1/state/%2/%3").arg(s->objectName(), mainWindow->getCurScreenLayout()).arg(pageNo);
        settings.getSettings().setValue(name, state);
    }
}
void KSTPageFrame::on_doSplitterChanges()
{
    getSplitters();
}
void KSTPageFrame::moveEvent(QMoveEvent *event)
{
    if (pageNo > 0)
    {
        RegSettings settings;
        settings.getSettings().setValue(QString("screen%1/geometry").arg(pageNo), saveGeometry());
    }
    QFrame::moveEvent(event);
}
void KSTPageFrame::resizeEvent(QResizeEvent * event)
{
    if (pageNo > 0)
    {
        RegSettings settings;
        settings.getSettings().setValue(QString("screen%1/geometry").arg(pageNo), saveGeometry());
    }
    QFrame::resizeEvent(event);
}
void KSTPageFrame::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        if (pageNo > 0)
        {
            RegSettings settings;
            settings.getSettings().setValue(QString("screen%1/geometry").arg(pageNo), saveGeometry());
        }
    }
    QFrame::changeEvent(e);
}

