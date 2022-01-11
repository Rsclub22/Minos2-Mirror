#include "base_pch.h"
#include "htmldelegate.h"
#include "contest.h"
#include "cutils.h"
#include "MinosLoggerEvents.h"
#include "MonitorMain.h"
#include "MonitoringFrame.h"
#include "ui_MonitoringFrame.h"

// When columns changed, need to tell ALL monitoring frames

MonitoringFrame::MonitoringFrame(MonitorMain *parent) :
    QFrame(parent), mparent(parent),
    ui(new Ui::MonitoringFrame)
{
    ui->setupUi(this);

    ui->QSOTable->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->QSOTable->horizontalHeader()->setSectionsMovable(true);

    connect( ui->QSOTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MonitoringFrame::onQSOTable_customContextMenuRequested );
    connect( ui->QSOTable->horizontalHeader(), &QHeaderView::sectionMoved, this, &MonitoringFrame::onQSOTable_sectionMoved);
    connect( ui->QSOTable->horizontalHeader(), &QHeaderView::sectionResized, this, &MonitoringFrame::onQSOTable_sectionResized);

    QSharedPointer<HtmlDelegate> delegate(new HtmlDelegate(1.0, 1.0));
    qsoModel.delegate = delegate;
    ui->QSOTable->setModel(&qsoModel);
    ui->QSOTable->setItemDelegate( delegate.data() );

    ui->QSOTable->verticalHeader()->setVisible(false);
    ui->QSOTable->setCornerButtonEnabled(false);
    ui->QSOTable->verticalHeader()->setDefaultSectionSize(10);
    ui->QSOTable->verticalHeader()->setMinimumSectionSize(10);
    ui->QSOTable->setAlternatingRowColors(true);

    ui->QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->QSOTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    createColumnsMenu(columnsMenu, ui->QSOTable->horizontalHeader(), this,
              [=]{
                    viewColumn();
              });

}
void MonitoringFrame::viewColumn()
{
    // a columnsMenu entry has been clicked... action it
    QAction *act = dynamic_cast<QAction *>(sender());
    if (act)
    {
        int col = act->data().toInt();
        if (col >= 0)
        {
            bool check = act->isChecked();
            ui->QSOTable->horizontalHeader()->setSectionHidden(col, !check);
        }
        else
        {
            QString fname("./Configuration/MonitorTableHeaders.ini");
            resetHeaderColumns(fname, "QSOTable", "", ui->QSOTable->horizontalHeader());
        }
    }
    saveQSOTableColumns();
}
void MonitoringFrame::saveQSOTableColumns()
{
    if (!inRestoreColumns)
    {
        QString fname("./Configuration/MonitorTableHeaders.ini");
        saveHeaderColumns(fname, "QSOTable", "", ui->QSOTable->horizontalHeader());
        MinosLoggerEvents::SendColumnsChanged();
    }
}
void MonitoringFrame::restoreQSOTableColumns()
{
    inRestoreColumns = true;
    QString fname("./Configuration/MonitorTableHeaders.ini");
    restoreHeaderColumns(fname, "QSOTable", "", ui->QSOTable->horizontalHeader());
    inRestoreColumns = false;
}
void MonitoringFrame::onQSOTable_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->QSOTable->mapToGlobal( pos );
    popupColumnsMenu(columnsMenu, globalPos, ui->QSOTable->horizontalHeader());
}
void MonitoringFrame::onQSOTable_sectionMoved(int, int, int)
{
    saveQSOTableColumns();
}
void MonitoringFrame::onQSOTable_sectionResized(int, int , int)
{
    saveQSOTableColumns();
}
MonitoringFrame::~MonitoringFrame()
{
    delete ui;
}
void MonitoringFrame::initialise( BaseContestLog * pcontest )
{
   contest = pcontest;
   qsoModel.initialise(contest);
}
void MonitoringFrame::showQSOs()
{
    restoreQSOTableColumns();
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
