/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Delete Voice/CW Memory Radios
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QMessageBox>
#include "deletedradioforvoicecwmemorybuttonsdialog.h"
#include "ui_deletedradioforvoicecwmemorybuttonsdialog.h"

DeletedRadioForVoiceCwMemoryButtonsDialog::DeletedRadioForVoiceCwMemoryButtonsDialog(QStringList listOfVoiceRadioNames, QStringList listOfCwRadioNames, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeletedRadioForVoiceCwMemoryButtonsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Delete Stored Radios - Voice or CW Memories"));

    ui->voiceMemoryRadioListWidget->addItems(listOfVoiceRadioNames);
    ui->voiceMemoryRadioListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    ui->cwMemoryRadioListWidget->addItems(listOfCwRadioNames);
    ui->cwMemoryRadioListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);



    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DeletedRadioForVoiceCwMemoryButtonsDialog::accepted);

    // Connect the "Cancel" button to close the dialog
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DeletedRadioForVoiceCwMemoryButtonsDialog::reject);

}

DeletedRadioForVoiceCwMemoryButtonsDialog::~DeletedRadioForVoiceCwMemoryButtonsDialog()
{
    delete ui;
}


QList<QListWidgetItem *> DeletedRadioForVoiceCwMemoryButtonsDialog::getVoiceSelectedItems()
{
    return ui->voiceMemoryRadioListWidget->selectedItems();
}


QList<QListWidgetItem *> DeletedRadioForVoiceCwMemoryButtonsDialog::getCwSelectedItems()
{
    return ui->cwMemoryRadioListWidget->selectedItems();
}


void DeletedRadioForVoiceCwMemoryButtonsDialog::accepted()
{
   voiceSelectedItems = getVoiceSelectedItems();
   cwSelectedItems = getCwSelectedItems();

   QString msgText;
   if (!voiceSelectedItems.isEmpty())
   {

        msgText.append("Voice Memory Radios:-\n");
        foreach (QListWidgetItem* item, voiceSelectedItems)
        {
            msgText.append(item->text() + "\n");
        }

        msgText.append("\n");

   }

   if (!cwSelectedItems.isEmpty())
   {

       msgText.append("Cw Memory Radios:-\n");
       foreach (QListWidgetItem* item, cwSelectedItems)
       {
           msgText.append(item->text() + "\n");
       }

       msgText.append("\n");
   }

   if (!voiceSelectedItems.isEmpty() || !cwSelectedItems.isEmpty())
   {

        int status = QMessageBox::question( this,
                                tr("Remove Radio"),
                                tr("Do you want to delete these radios?\n%1")
                                .arg(msgText),
                                QMessageBox::Yes|QMessageBox::No|QMessageBox::Escape,
                                QMessageBox::NoButton);




        if (status == QMessageBox::Yes)
        {
            accept();
        }
        else
        {
            reject();
        }
    }
    else
    {

        reject();
    }


}





