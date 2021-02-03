#include "base_pch.h"

#include <QSizeGrip>

#include "ContestApp.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "MinosLoggerEvents.h"
#include "qlogtabwidget.h"
#include "ContestPageControl.h"
#include "ContestPage.h"
#include "ui_ContestPageControl.h"

ContestPageControl::ContestPageControl(QWidget *parent) :
    QLogTabWidget(parent),
    ui(new Ui::ContestPageControl)
{
    ui->setupUi(this);
    setWindowTitle(tr("Minos Contest Logger"));
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect(&MinosLoggerEvents::mle, SIGNAL(ContestShownChanged()), this, SLOT(onContestShownChanged()));
    connect(this, SIGNAL(tabBarClicked(int)), this, SLOT(onTabBarClicked(int)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(onCustomContextMenuRequested(const QPoint)));
    connect(this, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(onTabBarDoubleClicked(int)));
}

ContestPageControl::~ContestPageControl()
{
    delete ui;
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
        QSettings settings;
        QByteArray geometry = settings.value(QString("screen%1/geometry").arg(getInstance())).toByteArray();
        if (geometry.size() > 0)
            restoreGeometry(geometry);

        tabBar()->setVisible(false);
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
        if (isMinimized())
        {
            ContestPage *ctab = dynamic_cast<ContestPage *>(currentWidget());

            setWindowTitle(ctab->pageName + tr(" (min)"));
        }
    }

    QLogTabWidget::changeEvent(e);
}


void ContestPageControl::onContestShownChanged()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();

    QMap<BaseContestLog *, ContestPage *>::iterator p = pages.find(ct);
    if (p == pages.end())
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

            BaseContestLog *pc = tslf->getContest();

            if (pc == ctab->getContest())
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
