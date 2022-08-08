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

   ui->QSOTable->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
   ui->QSOTable->horizontalHeader()->setSectionsMovable(true);

   connect( ui->QSOTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MonitoringFrame::onQSOTable_customContextMenuRequested );
   connect( ui->QSOTable->horizontalHeader(), &QHeaderView::sectionMoved, this, &MonitoringFrame::onQSOTable_sectionMoved);
   connect( ui->QSOTable->horizontalHeader(), &QHeaderView::sectionResized, this, &MonitoringFrame::onQSOTable_sectionResized);

   ui->QSOTable->setAlternatingRowColors(true);
   ui->QSOTable->verticalHeader()->setVisible(false);
   ui->QSOTable->setCornerButtonEnabled(false);
   ui->QSOTable->verticalHeader()->setVisible(false);
   ui->QSOTable->verticalHeader()->setDefaultSectionSize(1);
   ui->QSOTable->verticalHeader()->setMinimumSectionSize(1);
   ui->QSOTable->setWordWrap(false);
   ui->QSOTable->setCornerButtonEnabled(false);

   ui->QSOTable->horizontalHeader()->setHighlightSections(false);
   ui->QSOTable->horizontalHeader()->setStretchLastSection(true);
   ui->QSOTable->horizontalHeader()->setMinimumSectionSize(10);
   ui->QSOTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
   ui->QSOTable->horizontalHeader() ->setSectionsMovable( true );
   ui->QSOTable->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );

   QSharedPointer<HtmlDelegate> delegate(new HtmlDelegate(1.0, 1.0));
   qsoModel.delegate = delegate;

   qsoModel.initialise(contest);

   ui->QSOTable->setModel(&qsoModel);
   ui->QSOTable->setItemDelegate( delegate.data() );

   ui->QSOTable->setItemDelegate( delegate.data() );
   QSize ms = delegate->docSize("XX");
   ui->QSOTable->verticalHeader()->setDefaultSectionSize(ms.height() );
   ui->QSOTable->verticalHeader()->setMinimumSectionSize(10);

   ui->QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

   createColumnsMenu(columnsMenu, ui->QSOTable->horizontalHeader(), this,
             [=]{
                   viewColumn();
             });

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

void MonitoringFrame::on_monitorTimeout()
{
    if (newStanzas)
    {
        newStanzas = false;
        // only do this if the number of stanzas has changed
        if (rescanNeeded)
        {
            // clear dups here - we have no need of them in monitor
            getContest()->DupSheet.clear();
            getContest()->scanContest();  // this is MUCH too often... timer is 100ms!
            rescanNeeded = false;
        }
        setScore();

        ui->QSOTable->scrollToBottom();
    }
}
