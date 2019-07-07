#include "minoscontestloaddialog.h"
#include "ui_minoscontestloaddialog.h"

MinosContestLoadDialog::MinosContestLoadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MinosContestLoadDialog)
{
    ui->setupUi(this);
    el = new QEventLoop(this);
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, [=]()
    {
        // NB a lambda function
        timer->stop();
        el->quit();
    }
    );
    timer->start(500);
}

MinosContestLoadDialog::~MinosContestLoadDialog()
{
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

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}
void MinosContestLoadDialog::doShow()
{
    show();

    el->exec();
}
