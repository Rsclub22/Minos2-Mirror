#include "base_pch.h"

#include "ContestApp.h"
#include "list.h"
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


