#include "base_pch.h"

#include "ContestApp.h"
#include "list.h"
#include "tlogcontainer.h"
#include "tmanagelistsdlg.h"
#include "ui_tmanagelistsdlg.h"

TManageListsDlg::TManageListsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TManageListsDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QSettings settings;
    QByteArray geometry = settings.value("TManageListsDlg/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->ListsListBox->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    DrawList();
}

TManageListsDlg::~TManageListsDlg()
{
    delete ui;
}
void TManageListsDlg::doCloseEvent()
{
    QSettings settings;
    settings.setValue("TManageListsDlg/geometry", saveGeometry());
}
void TManageListsDlg::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TManageListsDlg::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void TManageListsDlg::DrawList()
{
    QString currList;
    int nlists = TContestApp::getContestApp() ->getListSlotCount();
    QList<QTableWidgetItem *> items = ui->ListsListBox->selectedItems();
    if (nlists && items.count() > 0)
    {
        int slotno = items[0]->data(Qt::UserRole).toInt();
        QSharedPointer<ListSlot> cs(TContestApp::getContestApp() ->listSlotList[ slotno ]);
        if (cs && cs->slot)
            currList = cs->slot->name;
    }

   ui->CloseListButton->setEnabled(false);
   ui->ListsListBox->clear();
   ui->ListsListBox->setColumnCount(1);

   int row = 0;
   int toSelect = 0;

   for ( auto const &l: qAsConst(TContestApp::getContestApp() ->listSlotList) )
   {
      if ( l&& l->slot )
      {
          ui->ListsListBox->setRowCount(row + 1);

          QTableWidgetItem * qlwi = new QTableWidgetItem();
          QString sname = l->slot->name;
          qlwi->setText( sname);
          qlwi->setData( Qt::UserRole, l->slotno);

          ui->ListsListBox->setItem(row, 0, qlwi );

          if (sname == currList)
          {
              toSelect = row;
          }

         ui->CloseListButton->setEnabled(true);
         row++;
      }
   }
   if ( row > 0 )
   {
       ui->ListsListBox->selectRow(toSelect);
   }
   enableActions();

}
void TManageListsDlg::enableActions()
{
    int tno = ui->ListsListBox->currentRow();

    ui->moveUpButton->setEnabled(tno > 0);
    ui->moveDownButton->setEnabled(tno >= 0 && tno < ui->ListsListBox->rowCount() - 1);

}
void TManageListsDlg::on_CloseListButton_clicked()
{
   QList<QTableWidgetItem *> items = ui->ListsListBox->selectedItems();
   if (items.count() <= 0)
       return;

   int slotno = items[0]->data(Qt::UserRole).toInt();
    for ( auto const &l: qAsConst(TContestApp::getContestApp() ->listSlotList ))
    {
       if ( l && l->slotno == slotno )
       {
          TContestApp::getContestApp() ->closeListFile( l ->slot );
          break;
       }
    }
    DrawList();
}

void TManageListsDlg::on_ExitButton_clicked()
{
    accept();
}



void TManageListsDlg::on_openListButton_clicked()
{
    LogContainer->doListOpenActionExecute(this);

    ui->ListsListBox->selectRow(TContestApp::getContestApp() ->listSlotList.count() - 1);

    DrawList();
}

void TManageListsDlg::on_moveUpButton_clicked()
{
    int tno = ui->ListsListBox->currentRow();
    if ( tno > 0 )
    {
       QTableWidgetItem *w0 = ui->ListsListBox->item(tno, 0);
       int s = w0->data(Qt::UserRole).toInt();
       QSharedPointer<ListSlot> cs = TContestApp::getContestApp() ->listSlotList[s];

       QTableWidgetItem *w1 = ui->ListsListBox->item(tno - 1, 0);
       int sm1 = w1->data(Qt::UserRole).toInt();
       QSharedPointer<ListSlot> csm1 = TContestApp::getContestApp() ->listSlotList[sm1];

       TContestApp::getContestApp() ->listSlotList[ s ] = csm1;
       csm1->slotno = s;


       TContestApp::getContestApp() ->listSlotList[ sm1 ] = cs;
       cs->slotno = sm1;

       TContestApp::getContestApp() ->writeListsList();

        ui->ListsListBox->selectRow(tno - 1);

       DrawList();
    }
}

void TManageListsDlg::on_moveDownButton_clicked()
{
    int tno = ui->ListsListBox->currentRow();
    if (tno >= 0 && tno < ui->ListsListBox->rowCount() - 1 )
    {

        QTableWidgetItem *w0 = ui->ListsListBox->item(tno, 0);
        int s = w0->data(Qt::UserRole).toInt();
        QSharedPointer<ListSlot> cs = TContestApp::getContestApp() ->listSlotList[s];
        QTableWidgetItem *w1 = ui->ListsListBox->item(tno + 1, 0);
        int sp1 = w1->data(Qt::UserRole).toInt();
        QSharedPointer<ListSlot> csp1 = TContestApp::getContestApp() ->listSlotList[sp1];

       TContestApp::getContestApp() ->listSlotList[ s ] = csp1;
       csp1->slotno = s;

       TContestApp::getContestApp() ->listSlotList[ sp1 ] = cs;
       cs->slotno = sp1;

       TContestApp::getContestApp() ->writeListsList();

       ui->ListsListBox->selectRow(tno + 1);

       DrawList();
    }
}

void TManageListsDlg::on_ListsListBox_itemSelectionChanged()
{
    enableActions();
}
