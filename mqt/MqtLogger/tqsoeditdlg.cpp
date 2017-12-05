#include "logger_pch.h"

#include "tqsoeditdlg.h"
#include "ui_tqsoeditdlg.h"

TQSOEditDlg *QSOEditDlg;

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
    ui(new Ui::TQSOEditDlg),
    firstContact( 0 ), contest( 0 ),
    unfilled(unfilled)
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

    ui->matchTreesFrame->setBaseName("Edit");
    ui->GJVQSOEditFrame->setAsEdit(true, "Edit");
    getSplitters();

    connect(ui->GJVQSOEditFrame, SIGNAL(QSOFrameCancelled()), this, SLOT(on_EditFrameCancelled()));
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)), this, SLOT(on_AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)));
    connect(ui->GJVQSOEditFrame, SIGNAL(xferPressed()), this, SLOT(onXferPressed()));
    connect(ui->matchTreesFrame, SIGNAL(xferPressed()), this, SLOT(onXferPressed()));
    connect(ui->matchTreesFrame, SIGNAL(setXferEnabled(bool)), ui->GJVQSOEditFrame, SLOT(setXferEnabled(bool)));

    ui->GJVQSOEditFrame->setXferEnabled(false);
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
       setWindowTitle("Completing unfilled contacts");
    }
    else
    {
       setWindowTitle("Editing QSO");
    }
    firstContact.reset();

    ui->matchTreesFrame->setContest(contest);

    int ret = QDialog::exec();

    MinosLoggerEvents::SendAfterLogContact(contest);

    return ret;
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

    ui->matchTreesFrame->getSplitters();
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
void TQSOEditDlg::onXferPressed()
{
   // transfer from current match
   if (!contest || contest->isReadOnly() )
      return ;

   MatchTreeItem *mi = ui->matchTreesFrame->getXferItem();

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
