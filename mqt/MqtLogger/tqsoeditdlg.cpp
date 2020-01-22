#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include "contest.h"
#include "MatchTreeFrame.h"

#include "tqsoeditdlg.h"
#include "ui_tqsoeditdlg.h"

//---------------------------------------------------------------------------
class QSOHistoryNode
{
   public:
      QSharedPointer<BaseContact> root;
      int historyOffset;
      QSOHistoryNode() :historyOffset( -1 )
      {}
};
//---------------------------------------------------------------------------
TQSOEditDlg::TQSOEditDlg(QWidget *parent, bool unfilled )
    :QDialog(parent),
    ui(new Ui::TQSOEditDlg)
  , contest( nullptr )
  , firstContact( nullptr )
  , unfilled(unfilled)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

#ifdef Q_OS_ANDROID
    splitterHandleWidth = 20;
#else
    splitterHandleWidth = 6;
#endif

    QSettings settings;
    QByteArray geometry = settings.value("QSOEditDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->thisMatchFrame->initialise();
    ui->otherMatchFrame->initialise();
    ui->archiveMatchFrame->initialise();

    OtherMatchTreeFW = new FocusWatcher(ui->otherMatchFrame->getTreeView());
    connect(OtherMatchTreeFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(onOtherMatchTreeFocused(QObject *, bool, QFocusEvent *)));
    ArchiveMatchTreeFW = new FocusWatcher(ui->archiveMatchFrame->getTreeView());
    connect(ArchiveMatchTreeFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(onArchiveTreeFocused(QObject *, bool, QFocusEvent *)));

    ui->thisMatchFrame->setBaseName("Edit");
    ui->otherMatchFrame->setBaseName("Edit");
    ui->archiveMatchFrame->setBaseName("Edit");

    ui->GJVQSOEditFrame->setAsEdit(true, "Edit");
    getSplitters();

    connect(ui->GJVQSOEditFrame, SIGNAL(QSOFrameCancelled()), this, SLOT(on_EditFrameCancelled()));
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)), this, SLOT(on_AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)));
    connect(&MinosLoggerEvents::mle, SIGNAL(MatchStarting(BaseContestLog*)), this, SLOT(on_MatchStarting(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(XferPressed(BaseContestLog *, QString)), this, SLOT(onXferPressed(BaseContestLog *, QString)));
    connect(&MinosLoggerEvents::mle, SIGNAL(XferEnabled(bool, BaseContestLog *, QString)), ui->GJVQSOEditFrame, SLOT(setXferEnabled(bool, BaseContestLog *, QString)));
    connect(&MinosLoggerEvents::mle, SIGNAL(MatchTreeSelected(MatchType , BaseContestLog *, QString, QItemSelection)),
            this, SLOT(MatchTreeSelected(MatchType, BaseContestLog *, QString, QItemSelection)));

    ui->GJVQSOEditFrame->setXferEnabled(false, contest, "Edit");
}
TQSOEditDlg::~TQSOEditDlg()
{
    delete ui;
}
int TQSOEditDlg::exec()
{
    ui->GJVQSOEditFrame->unfilled = unfilled;

    ui->GJVQSOEditFrame->initialise( contest );

    ui->GJVQSOEditFrame->selectEntryForEdit( firstContact );   // first contact for the dialog to deal with
    if ( unfilled )
    {
       setWindowTitle(tr("Completing unfilled contacts"));
    }
    else
    {
       setWindowTitle(tr("Editing QSO"));
    }
    firstContact.reset();

    ui->thisMatchFrame->setContest(contest);
    ui->otherMatchFrame->setContest(contest);
    ui->archiveMatchFrame->setContest(contest);

    int ret = QDialog::exec();

    MinosLoggerEvents::SendAfterLogContact(contest);

    return ret;
}

void TQSOEditDlg::onOtherMatchTreeFocused(QObject *, bool in, QFocusEvent * )
{
    if (!in)
    {
        ui->archiveMatchFrame->getTreeView()->viewport()->update();
        ui->otherMatchFrame->getTreeView()->viewport()->update();
        return;
    }

    xferTree = ui->otherMatchFrame;
    ui->otherMatchFrame->setCurrentModel(true);
    ui->archiveMatchFrame->setCurrentModel(false);

    ui->archiveMatchFrame->getTreeView()->viewport()->update();
}
void TQSOEditDlg::onArchiveTreeFocused(QObject *, bool in, QFocusEvent * )
{
    if (!in)
    {
        ui->archiveMatchFrame->getTreeView()->viewport()->update();
        ui->otherMatchFrame->getTreeView()->viewport()->update();
        return;
    }

    xferTree = ui->archiveMatchFrame;
    ui->archiveMatchFrame->setCurrentModel(true);
    ui->otherMatchFrame->setCurrentModel(false);

     ui->otherMatchFrame->getTreeView()->viewport()->update();
}
//---------------------------------------------------------------------------
void TQSOEditDlg::keyPressEvent( QKeyEvent* event )
{
    ui->GJVQSOEditFrame->doKeyPressEvent(event);
}
void TQSOEditDlg::on_EditFrameCancelled()
{
    accept();
}
void TQSOEditDlg::getSplitters()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("editSplitter/state").toByteArray();
    ui->editSplitter->restoreState(state);
    ui->editSplitter->setHandleWidth(splitterHandleWidth);

    state = settings.value("Log/ArchiveSplitter/state").toByteArray();
    ui->archiveSplitter->restoreState(state);
    ui->archiveSplitter->setHandleWidth(splitterHandleWidth);
}
void TQSOEditDlg::on_editSplitter_splitterMoved(int, int)
{
    QByteArray state = ui->editSplitter->saveState();
    QSettings settings;
    settings.setValue("editSplitter/state", state);
}
//---------------------------------------------------------------------------

void TQSOEditDlg::selectContact( BaseContestLog * ccontest, QSharedPointer<BaseContact> lct )
{
   // this is the first call after construction
   contest = ccontest;
   firstContact = lct;
}

void TQSOEditDlg::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ui->GJVQSOEditFrame->valid(cmCheckValid);
}
//---------------------------------------------------------------------------
void TQSOEditDlg::addTreeRoot(QSharedPointer<BaseContact> lct)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->QSOHistoryTree);

    QString line = lct->updtime.getDate( DTGACCURATE ) + " " + lct->updtime.getTime( DTGACCURATE ) + " UTC";
    treeItem->setText(0, line);

    lct->getText(line, contest);
    addTreeChild(treeItem, line);
    treeItem->setExpanded(true);
}

void TQSOEditDlg::addTreeChild(QTreeWidgetItem *parent,
                  QString text)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();

    treeItem->setText(0, text);

    parent->addChild(treeItem);
}
void TQSOEditDlg::on_AfterSelectContact(QSharedPointer<BaseContact> lct, BaseContestLog * /*contest*/)
{
  ui->QSOHistoryTree->clear();
  if (lct)
  {
      for (int i = 0; i < lct->getHistory().size(); ++i)
      {
          addTreeRoot(lct->getHistory()[i]);
      }
  }
  refreshOps(ui->GJVQSOEditFrame->screenContact);
}
//---------------------------------------------------------------------------
void TQSOEditDlg::refreshOps( ScreenContact &screenContact )
{
    ui->GJVQSOEditFrame->refreshOps(screenContact);
}
//---------------------------------------------------------------------------
void TQSOEditDlg::doCloseEvent()
{
    QSettings settings;
    settings.setValue("QSOEditDialog/geometry", saveGeometry());
}
void TQSOEditDlg::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TQSOEditDlg::accept()
{
    doCloseEvent();
    QDialog::accept();
}
void TQSOEditDlg::on_MatchStarting(BaseContestLog *ct)
{
      // clear down match trees
    if (contest == ct)
    {
      xferTree = nullptr;

      ui->thisMatchFrame->treeClickIndex = QModelIndex();
      ui->otherMatchFrame->treeClickIndex = QModelIndex();
      ui->archiveMatchFrame->treeClickIndex = QModelIndex();

      ui->GJVQSOEditFrame->setXferEnabled(false, contest, "Edit");
    }
}

MatchTreeItem * TQSOEditDlg::getXferItem()
{
   // transfer from current match

   // copy relevant parts of match contact to screen contact
   if (ui->archiveMatchFrame->treeClickIndex.isValid() && ( xferTree == nullptr || ui->archiveMatchFrame == xferTree) )
   {
      MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * >(ui->archiveMatchFrame->treeClickIndex.internalPointer());

      return MatchTreeIndex;

   }
   else
   {
      if (ui->otherMatchFrame->treeClickIndex.isValid() && ( xferTree == nullptr || ui->otherMatchFrame == xferTree) )
      {
         MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * > (ui->otherMatchFrame->treeClickIndex.internalPointer());

         return  MatchTreeIndex;
      }
   }
   return nullptr;
}
void TQSOEditDlg::onXferPressed(BaseContestLog *c, QString b)
{
   // transfer from current match
   if (!contest || contest->isReadOnly() || c != contest || b != "Edit" )
      return ;

   MatchTreeItem *mi = getXferItem();

   if (mi)
       transferDetails(mi);
}
//==============================================================================
void TQSOEditDlg::transferDetails(MatchTreeItem *MatchTreeIndex )
{
    if ( !contest  )
    {
       return ;
    }
   // needs to be transferred into QSOLogFrame.cpp
   QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();

   if (mc)
   {
       QSharedPointer<BaseContact> bct = mc->getBaseContact();

       if ( bct )
       {
          BaseContestLog *matct = mc->getContactLog();
          ui->GJVQSOEditFrame->transferDetails( bct, matct );
       }
       else
       {
           ListContact *lct = mc->getListContact();
           if (lct)
           {
               ContactList *matct = mc->getContactList();
               ui->GJVQSOEditFrame->transferDetails( lct, matct );
           }
       }
   }
}

void TQSOEditDlg::on_archiveSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->archiveSplitter->saveState();
    QSettings settings;
    settings.setValue("Edit/ArchiveSplitter/state", state);
    MinosLoggerEvents::SendSplittersChanged();
}
void TQSOEditDlg::MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, const QItemSelection &/*selected*/)
{
    if (contest == c && basename == "Edit")
    {
        switch (m)
        {
        case ThisMatch:
            //xferTree =  ui->thisMatchFrame;
            break;

        case OtherMatch:
            xferTree = ui->otherMatchFrame;
            break;

        case ArchiveMatch:
            xferTree = ui->archiveMatchFrame;
            break;
        }
    }
}
