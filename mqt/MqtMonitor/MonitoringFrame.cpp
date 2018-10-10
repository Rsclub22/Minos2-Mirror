#include "base_pch.h"
#include "htmldelegate.h"
#include "contest.h"
#include "MonitorMain.h"
#include "MonitoringFrame.h"
#include "ui_MonitoringFrame.h"

MonitoringFrame::MonitoringFrame(MonitorMain *parent) :
    QFrame(parent), mparent(parent),
    ui(new Ui::MonitoringFrame)
{
    ui->setupUi(this);
    ui->QSOTable->setItemDelegate( new HtmlDelegate(1.7, 0.5) );

    ui->QSOTable->resizeColumnsToContents();
    ui->QSOTable->resizeRowsToContents();

    ui->QSOTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    connect( ui->QSOTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));

}

MonitoringFrame::~MonitoringFrame()
{
    delete ui;
}
void MonitoringFrame::initialise( BaseContestLog * pcontest )
{
    // we don't currently have the required structures to include here...
   contest = pcontest;
   qsoModel.initialise(contest);
   ui->QSOTable->setModel(&qsoModel);

}
void MonitoringFrame::showQSOs()
{
    restoreColumns();
}
void MonitoringFrame::setScore()
{
    QString statbuf;
    if (contest)
        contest->setScore( statbuf );
    ui->scoreLabel->setText(statbuf);
}

void MonitoringFrame::update()
{
    qsoModel.reset();
}
void MonitoringFrame::on_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;

    state = ui->QSOTable->horizontalHeader()->saveState();
    settings.setValue("QSOTable/state", state);
}
void MonitoringFrame::restoreColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("QSOTable/state").toByteArray();
    ui->QSOTable->horizontalHeader()->restoreState(state);


}
