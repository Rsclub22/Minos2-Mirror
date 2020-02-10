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
    ui->ListsListBox->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    DrawList();
}

TManageListsDlg::~TManageListsDlg()
{
    delete ui;
}
void TManageListsDlg::DrawList()
{
   ui->CloseListButton->setEnabled(false);
   ui->ListsListBox->clear();
   ui->ListsListBox->setColumnCount(1);

   int row = 0;

   for ( ListSlotIterator i = TContestApp::getContestApp() ->listSlotList.begin(); i != TContestApp::getContestApp() ->listSlotList.end(); i++ )
   {
      if ( ( *i ) && ( *i ) ->slot )
      {
          ui->ListsListBox->setRowCount(row + 1);

          QTableWidgetItem * qlwi = new QTableWidgetItem();
          qlwi->setText( ( *i ) ->slot->name );
          qlwi->setData( Qt::UserRole, (*i)->slotno);

          ui->ListsListBox->setItem(row, 0, qlwi );

         ui->CloseListButton->setEnabled(true);
         row++;
      }
   }
   if ( row > 0 )
   {
       enableActions();
      //ui->ListsListBox->setItemIndex(0);
   }
   else
      reject();
}
void TManageListsDlg::on_CloseListButton_clicked()
{
   QList<QTableWidgetItem *> items = ui->ListsListBox->selectedItems();
   if (items.count() <= 0)
       return;

   int slotno = items[0]->data(Qt::UserRole).toInt();
    for ( ListSlotIterator i = TContestApp::getContestApp() ->listSlotList.begin(); i != TContestApp::getContestApp() ->listSlotList.end(); i++ )
    {
       if ( ( *i ) && ( *i ) ->slotno == slotno )
       {
          TContestApp::getContestApp() ->closeListFile( ( *i ) ->slot );
//          ( *i ) ->slot = 0;
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
    LogContainer->ListOpenActionExecute();
    DrawList();
}

void TManageListsDlg::enableActions()
{

}
void TManageListsDlg::on_moveUpButton_clicked()
{
    int tno = ui->ListsListBox->currentRow();
    if ( tno > 0 )
    {
       QSharedPointer<ListSlot> cs = TContestApp::getContestApp() ->listSlotList[ tno ];
       int s = cs->slotno;
       QSharedPointer<ListSlot> csm1 = TContestApp::getContestApp() ->listSlotList[ tno - 1 ];
       int sm1 = csm1->slotno;
       TContestApp::getContestApp() ->listSlotList[ tno ] = csm1;
       csm1->slotno = s;

       TContestApp::getContestApp() ->listSlotList[ tno - 1 ] = cs;
       cs->slotno = sm1;

       TContestApp::getContestApp() ->writeListsList();

       DrawList();
    }
}

void TManageListsDlg::on_moveDownButton_clicked()
{

}
