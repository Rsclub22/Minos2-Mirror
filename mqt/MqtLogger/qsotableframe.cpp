#include "AppStartup.h"
#include "ContestApp.h"
#include "MinosLoggerEvents.h"
#include "MTrace.h"
#include "contest.h"
#include "cutils.h"
#include "delayedaction.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"

#include "qsotableframe.h"
#include "ui_qsotableframe.h"

QSOTableFrame::QSOTableFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::QSOTableFrame)
{
    ui->setupUi(this);

    QVBoxLayout *ListV = new QVBoxLayout(this);
    ListV->setSpacing(0);
    ListV->setObjectName(QStringLiteral("verticalLayout_5"));
    ListV->setContentsMargins(0, 0, 0, 0);

    QSOTable = new MinosTableView(this);
    QSOTable->setObjectName(QStringLiteral("QSOTable"));
    QSOTable->setFocusPolicy(Qt::ClickFocus);
    QSOTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QSOTable->setAlternatingRowColors(true);
    QSOTable->setSelectionMode(QAbstractItemView::SingleSelection);
    QSOTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    QSOTable->setWordWrap(false);
    QSOTable->setCornerButtonEnabled(false);

    QSOTable->verticalHeader()->setVisible(false);
    QSOTable->verticalHeader()->setMinimumSectionSize(1);
    QSOTable->verticalHeader()->setDefaultSectionSize(1);

    QSOTable->horizontalHeader()->setHighlightSections(false);
    QSOTable->horizontalHeader()->setStretchLastSection(true);
    QSOTable->horizontalHeader()->setMinimumSectionSize(10);
    QSOTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    QSOTable->horizontalHeader()->setSectionsMovable( true );
    QSOTable->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0));

    qsoModel.delegate = delegate;

    QSOTable->setItemDelegate( delegate.data() );
    QSize ms = delegate->docSize("XX");
    QSOTable->verticalHeader()->setDefaultSectionSize(ms.height() );
    QSOTable->verticalHeader()->setMinimumSectionSize(10);

    connect( QSOTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &QSOTableFrame::onQSOGrid_customContextMenuRequested );
    connect( QSOTable->horizontalHeader(), &QHeaderView::sectionMoved, this, &QSOTableFrame::onQSOGrid_sectionMoved);

    ListV->addWidget(QSOTable);
}

QSOTableFrame::~QSOTableFrame()
{
    delete ui;
}
void QSOTableFrame::setContest(BaseContestLog *ct)
{
    contest = ct;
    qsoModel.setContest(contest);
    QSOTable->setModel(&qsoModel);
}

void QSOTableFrame::setCurScreenLayout(const QString &value)
{
    curScreenLayout = value;
}
void QSOTableFrame::buildFrame()
{
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ColumnsChanged, this, &QSOTableFrame::onColumnsChanged);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::AfterSelectContact, this, &QSOTableFrame::on_AfterSelectContact, Qt::QueuedConnection);

    connect( QSOTable->horizontalHeader(), &QHeaderView::sectionResized, this, &QSOTableFrame::on_sectionResized);
    connect(QSOTable, &QTableView::doubleClicked, this, &QSOTableFrame::onQSOTable_doubleClicked);

    createColumnsMenu(columnsMenu, QSOTable->horizontalHeader(), this,
                      [=]{
                          viewColumn();
                      });

    restoreQSOTableColumns();

    QSOTable->setItemDelegate( delegate.data() );
    QSize ms = delegate->docSize("XX");
    QSOTable->verticalHeader()->setDefaultSectionSize(ms.height());
    QSOTable->verticalHeader()->setMinimumSectionSize(10);

    QSOTable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

}
void QSOTableFrame::QSOTreeSelectContact( QSharedPointer<BaseContact> lct )
{
    if (lct)
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        if ( tslf )
        {
            tslf->EditContact( lct.data(), false );
        }
    }
}
void QSOTableFrame::on_AfterSelectContact( QSharedPointer<BaseContact>lct, BaseContestLog *ct)
{
    if (ct == contest && !lct)
    {
        // use a lambda on a short timer as when contest is first opened, it doesn't actually scroll
        delayedAction(this, [=]()
                      {
                          // NB a lambda function
                          QSOTable->scrollToBottom();
                          int row = QSOTable->model()->rowCount() - 1;
                          if (row >= 0)
                          {
                              QModelIndex oldIndex = QSOTable->currentIndex();
                              if (oldIndex.row() != row)
                              {
                                  QModelIndex index = QSOTable->model()->index( row, 0 );
                                  QSOTable->setCurrentIndex(index);
                              }
                          }
                      }
                      );
    }
}

void QSOTableFrame::setModel()
{
    // qsoModel.initialise(contest);
    // QSOTable->setModel(&qsoModel);
}

void QSOTableFrame::refreshModel()
{
    qsoModel.reset();
}

void QSOTableFrame::restoreQSOTableColumns()
{
    inRestoreColumns = true;
    QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
    restoreHeaderColumns(fname, "QSOTable", curScreenLayout, QSOTable->horizontalHeader());

    columnsChanged = false;
    inRestoreColumns = false;
}
void QSOTableFrame::saveQSOTableColumns()
{
    if (!inRestoreColumns)
    {
        QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
        saveHeaderColumns(fname, "QSOTable", curScreenLayout, QSOTable->horizontalHeader());

        MinosLoggerEvents::SendColumnsChanged();
    }
}
void QSOTableFrame::startNextEntry()
{
    restoreQSOTableColumns();
    columnsChanged = false;
}
void QSOTableFrame::onContestChanged()
{
    if ( columnsChanged )
    {
        MinosLoggerEvents::SendDoColumnChanges(contest);             // this does a restorePartial in showQSOs
        columnsChanged = false;
    }

}

void QSOTableFrame::insertRow(int rowNum)
{
    QSOTable->model()->insertRows(rowNum, 1, QModelIndex());
}
void QSOTableFrame::onQSOTable_doubleClicked(const QModelIndex &index)
{
    QSOTreeSelectContact(contest->pcontactAt( index.row() ));
}

void QSOTableFrame::on_sectionResized(int a, int b, int c)
{
    trace(QString("TSingleLogFrame::on_sectionResized %1 %2 %3").arg(a).arg(b).arg(c));
    saveQSOTableColumns();
}

void QSOTableFrame::onColumnsChanged()
{
    columnsChanged = true;
}
void QSOTableFrame::onQSOGrid_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = QSOTable->mapToGlobal( pos );
    popupColumnsMenu(columnsMenu, globalPos, QSOTable->horizontalHeader());
}
void QSOTableFrame::viewColumn()
{
    // a columnsMenu entry has been clicked... action it
    QAction *act = dynamic_cast<QAction *>(sender());
    if (act)
    {
        int col = act->data().toInt();
        if (col >= 0)
        {
            bool check = act->isChecked();
            QSOTable->horizontalHeader()->setSectionHidden(col, !check);
        }
        else
        {
            QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
            resetHeaderColumns(fname, "QSOTable", curScreenLayout, QSOTable->horizontalHeader());
        }
    }
    trace("TSingleLogFrame::viewColumn()");
    saveQSOTableColumns();
}
void QSOTableFrame::onQSOGrid_sectionMoved(int, int, int)
{
    trace("TSingleLogFrame::onQSOGrid_sectionMoved");
    saveQSOTableColumns();
}

