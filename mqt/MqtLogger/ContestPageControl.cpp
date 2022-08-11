#include <QSizeGrip>
#include <QToolTip>
#include "WindowsAppId.h"
#include "SecondInstall.h"

#include "ContestApp.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "MinosLoggerEvents.h"
#include "LoggerContest.h"
#include "qlogtabwidget.h"
#include "ContestPage.h"
#include "MTrace.h"

#include "ContestPageControl.h"
#include "ui_ContestPageControl.h"

ContestPageControl::ContestPageControl(QWidget *parent) :
    QLogTabWidget(parent),
    ui(new Ui::ContestPageControl)
{
    ui->setupUi(this);

    setWindowTitle(tr("Minos Contest Logger"));
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ContestShownChanged, this, &ContestPageControl::onContestShownChanged);
    connect(this, &ContestPageControl::tabBarClicked, this, &ContestPageControl::onTabBarClicked);
    connect(this, &ContestPageControl::customContextMenuRequested, this, &ContestPageControl::onCustomContextMenuRequested);
    connect(this, &ContestPageControl::tabBarDoubleClicked, this, &ContestPageControl::onTabBarDoubleClicked);

    tabBar()->installEventFilter(this);

}
ContestPageControl::~ContestPageControl()
{
    delete ui;
}
bool ContestPageControl::eventFilter(QObject */*obj*/, QEvent *event)
{
    if (event->type() == QEvent::Close)
    {
        if (instance > 0)
        {
            clearWinAppId(this);
        }
    }
    else if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        int curtab = tabBar()->tabAt(helpEvent->pos());
        if (curtab >= 0)
        {
            ContestPage *ctab = dynamic_cast<ContestPage *>(widget(curtab));
            BaseContestLog *pc = ctab->getContest();
            QString statbuf;
            if ( pc )
            {
                pc->setScore( statbuf );
                QString toolTip = pc->cfileName + "\r\n" + statbuf;
                if (!pc->isReadOnly())
                {
                    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( pc);

                    memoryData::memData m;
                    if (ct->runMemories.size() > 0 && ct->runMemories[ct->currentBand.getValue()].size() > 0)
                    {
                       m = ct->runMemories[ct->currentBand.getValue()][0].getValue();
                       Frequency cq = m.freq;
                       if (!cq.isClear())
                       {
                            toolTip += "\r\n" + tr("Run Frequency 1") + " " + cq.convertFreqStrDisp();
                       }

                    }
                    if (ct->runMemories.size() > 0 && ct->runMemories[ct->currentBand.getValue()].size() > 1)
                    {
                       m = ct->runMemories[ct->currentBand.getValue()][1].getValue();
                       Frequency cq = m.freq;
                       if (!cq.isClear())
                       {
                            toolTip += "\r\n" + tr("Run Frequency 2") + " " + cq.convertFreqStrDisp();
                       }

                    }

                }
                QToolTip::showText(helpEvent->globalPos(), toolTip);
            }
            else
            {
                QToolTip::hideText();
                event->ignore();
            }

        }
        else
        {
            QToolTip::hideText();
            event->ignore();
        }

        return true;
    }

    return false;
}

int ContestPageControl::getInstance() const
{
    return instance;
}

void ContestPageControl::setInstance(int value)
{
    instance = value;

    if (instance > 0)
    {
        // we seem to have to do the geometry before the appId stuff,
        // or the geometry stuff doesn't work - don't know why!

        QSettings settings;
        QByteArray geometry = settings.value(QString("screen%1/geometry").arg(instance)).toByteArray();
        if (geometry.size() > 0)
        {
            restoreGeometry(geometry);
        }

        tabBar()->setVisible(false);
        bool sep = false;
        TContestApp::getContestApp() ->getBoolDisplayProfile( edpSeparateIcons, sep );

        int subinst = 0;
        if (sep)
            subinst = instance;

        // the string shoud be formed from "CompanyName.ProductName.SubProduct.VersionInformation"
        // cf https://docs.microsoft.com/en-us/windows/win32/shell/appids

        setWinAppId(this, SecondInstall::getOrgName() + QString(".MqtLogger.SubScreen%1").arg(subinst) );
    }
}

void ContestPageControl::moveEvent(QMoveEvent *event)
{
    if (instance > 0)
    {
        QSettings settings;
        settings.setValue(QString("screen%1/geometry").arg(getInstance()), saveGeometry());
    }
    QLogTabWidget::moveEvent(event);
}
void ContestPageControl::resizeEvent(QResizeEvent * event)
{
    if (instance > 0)
    {
        QSettings settings;
        settings.setValue(QString("screen%1/geometry").arg(getInstance()), saveGeometry());
    }
    QLogTabWidget::resizeEvent(event);
}
void ContestPageControl::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        if (instance > 0)
        {
            QSettings settings;
            settings.setValue(QString("screen%1/geometry").arg(getInstance()), saveGeometry());
        }
        ContestPage *ctab = dynamic_cast<ContestPage *>(currentWidget());
        if (ctab)
        {
            if (isMinimized())
            {
                setWindowTitle(ctab->pageName + tr(" (min)"));
            }
            else
            {
                setWindowTitle(ctab->pageName);
            }
        }
    }

    QLogTabWidget::changeEvent(e);
}


void ContestPageControl::onContestShownChanged()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();

    QMap<BaseContestLog *, ContestPage *>::iterator p = pages.find(ct);
    if (!(*p) || p == pages.end())
    {
        trace(QString("Hide CPC %1 %2").arg(instance).arg(ct->name.getValue()));
        hide();
    }
    else
    {
        setCurrentWidget(*p);
        trace(QString("Show CPC %1 %2").arg(instance).arg(ct->name.getValue()));
        if (instance == 0 || (*p)->hasElements())
        {
            // we always want TLogContainer shown, or we can't do much
            show();
        }
        else
        {
            hide();
        }
    }

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if ( tslf )
    {
        for (int i = 0; i < count(); i++)
        {
            ContestPage *ctab = dynamic_cast<ContestPage *>(widget(i));

            BaseContestLog *pc = ctab->getContest();

            if (pc == tslf->getContest())
            {
                setWindowTitle(ctab->pageName);
                trace(QString("setWindowTitle %1").arg(ctab->pageName));

                setTabColor(i, Qt::red);
                if (ctab == tslf)
                {
                    LogContainer->setMenuLog(i);
                }
            }
            else if (pc->isReadOnly())
            {
                setTabColor(i, Qt::darkGreen);
            }
            else
            {
                setTabColor(i, Qt::darkBlue);
            }
        }
    }
    else
    {
        setWindowTitle(tr("Minos Contest Logger"));
    }
}
void ContestPageControl::onTabBarClicked(int index)
{
    trace(QString("onTabBarClicked %1").arg(index));
    ContestPage *ctab = dynamic_cast<ContestPage *>(widget(index));
    BaseContestLog *pc = ctab->getContest();

    LogContainer->selectContest(pc);
}
void ContestPageControl::onCustomContextMenuRequested(const QPoint &pos)
{
    LogContainer->setMemoryAction->setVisible(false);

    int curtab = tabBar()->tabAt(pos);
    if (curtab >= 0)
    {
        ContestPage *ctab = dynamic_cast<ContestPage *>(widget(curtab));
        BaseContestLog *pc = ctab->getContest();

        LogContainer->selectContest(pc);
    }

    QPoint globalPos = mapToGlobal( pos );

    QApplication *qa = dynamic_cast<QApplication *>(QApplication::instance());
    QObject *w = qa->widgetAt(globalPos);

    while (w)
    {
        MatchTreeFrame *mtf = dynamic_cast<MatchTreeFrame *>(w);

        if (mtf)
        {
            mtf->doCustomContextMenuRequested();
            break;
        }

        w = w->parent();
    }

    LogContainer->TabPopup.popup( globalPos );
}

void ContestPageControl::onTabBarDoubleClicked(int /*index*/)
{
    LogContainer->ContestDetailsActionExecute();
}
