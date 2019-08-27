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
   contest = pcontest;
   qsoModel.initialise(contest);
   QSharedPointer<HtmlDelegate> delegate(new HtmlDelegate(1.0, 1.0));
   qsoModel.delegate = delegate;
   ui->QSOTable->setModel(&qsoModel);

   ui->QSOTable->verticalHeader()->setVisible(false);
   ui->QSOTable->setCornerButtonEnabled(false);
   ui->QSOTable->verticalHeader()->setDefaultSectionSize(10);
   ui->QSOTable->verticalHeader()->setMinimumSectionSize(10);
   ui->QSOTable->setAlternatingRowColors(true);
   ui->QSOTable->setItemDelegate( delegate.data() );

   ui->QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

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
