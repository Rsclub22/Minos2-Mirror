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
    connect(&MinosLoggerEvents::mle, SIGNAL(ContestShownChanged()), this, SLOT(on_ContestShownChanged()));
    connect(this, SIGNAL(tabBarClicked(int)), this, SLOT(on_TabBarClicked(int)));
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
    }

    QLogTabWidget::changeEvent(e);
}


void ContestPageControl::on_ContestShownChanged()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();

    QMap<BaseContestLog *, ContestPage *>::iterator p = pages.find(ct);
    if (p == pages.end())
    {
        trace(QString("Hide CPC %1").arg(instance));
        hide();
    }
    else
    {
        setCurrentWidget(*p);
        trace(QString("Show CPC %1").arg(instance));
        show();
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
}
void ContestPageControl::on_TabBarClicked(int index)
{
    trace(QString("on_TabBarClicked %1").arg(index));
    ContestPage *ctab = dynamic_cast<ContestPage *>(widget(index));
    BaseContestLog *pc = ctab->getContest();

    LogContainer->selectContest(pc);
}
