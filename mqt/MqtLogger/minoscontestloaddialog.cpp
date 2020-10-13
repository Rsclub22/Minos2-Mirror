#include "base_pch.h"
#include "delayedaction.h"
#include "ContestApp.h"

#include "minoscontestloaddialog.h"
#include "ui_minoscontestloaddialog.h"

MinosContestLoadDialog::MinosContestLoadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MinosContestLoadDialog)
{

    trace("Progress Dialog create");

    ui->setupUi(this);
    el = new QEventLoop(this);
    //timer = new QTimer(this);
}

MinosContestLoadDialog::~MinosContestLoadDialog()
{
    trace("Progress Dialog destructor");

    delete ui;
    //timer->stop();
    //timer->deleteLater();
    el->quit();
    el->deleteLater();
}

void MinosContestLoadDialog::setLoadMessage(QString mess, bool newFile, bool list)
{
    QString m = "<h2><center>";

    m += newFile?tr("Creating "):tr("Loading ");

    m += list?tr("List file "):tr("Contest file ");
    m += "<p>";
    m += mess;

    ui->contestNameLabel->setText(m);

    trace("Progress Dialog add message for " + mess);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}
void MinosContestLoadDialog::doShow()
{
    trace("Progress Dialog doShow");
    show();

    el->exec();
    trace("Exit from doShow after delay for screen update");
}

void MinosContestLoadDialog::showEvent(QShowEvent *ev)
{
    QDialog::showEvent(ev);
    if (isVisible())
    {
        int progDelay;
        TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpProgressDelay, progDelay );

        delayedAction(this, [=]()
        {
            // NB a lambda function
            trace("Progress Dialog timer fired");
            //timer->stop();
            el->quit();
        }
        ,progDelay
        );
    }
}
