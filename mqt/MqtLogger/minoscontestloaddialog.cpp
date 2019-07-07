#include "base_pch.h"

#include "minoscontestloaddialog.h"
#include "ui_minoscontestloaddialog.h"

MinosContestLoadDialog::MinosContestLoadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MinosContestLoadDialog)
{

    trace("Progress Dialog create");

    ui->setupUi(this);
    el = new QEventLoop(this);
    timer = new QTimer(this);
}

MinosContestLoadDialog::~MinosContestLoadDialog()
{
    trace("Progress Dialog destructor");

    delete ui;
    timer->stop();
    timer->deleteLater();
    el->quit();
    el->deleteLater();
}

void MinosContestLoadDialog::setLoadMessage(QString mess, bool newFile, bool list)
{
    QString m = "<h2><center>";

    m += newFile?"Creating ":"Loading ";

    m += list?"List file ":"Contest file ";
    m += "<p>";
    m += mess;

    ui->contestNameLabel->setText(m);

    trace("Progress Dialog add mesage for " + mess);

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}
void MinosContestLoadDialog::doShow()
{
    trace("Progress Dialog doShow");
    show();

    connect(timer, &QTimer::timeout, [=]()
    {
        // NB a lambda function
        trace("Progress Dialog timer fired");
        timer->stop();
        el->quit();
    }
    );

    timer->start(500);
    el->exec();
}
