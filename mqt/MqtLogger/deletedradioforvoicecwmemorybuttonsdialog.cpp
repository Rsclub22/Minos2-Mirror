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

DeletedRadioForVoiceCwMemoryButtonsDialog::DeletedRadioForVoiceCwMemoryButtonsDialog(QStringList listOfRadioNames,  VoiceKeyerId id_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeletedRadioForVoiceCwMemoryButtonsDialog)
{
    ui->setupUi(this);

    id = id_;

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (id == VoiceKeyerId::RigControl)
    {
        setWindowTitle(tr("Delete RigControl Radios - Voice Keyer Memories"));
        ui->cwMemoryRadioListWidget->setVisible(false);
        ui->voiceMemoryRadioListWidget->setVisible(true);
        ui->voiceMemoryRadioListWidget->addItems(listOfRadioNames);
        ui->voiceMemoryRadioListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    else if (id == VoiceKeyerId::CW_RigControl)
    {
        setWindowTitle(tr("Delete RigControl Radios - CW Keyer Memories"));
        ui->cwMemoryRadioListWidget->setVisible(true);
        ui->voiceMemoryRadioListWidget->setVisible(false);
        ui->cwMemoryRadioListWidget->addItems(listOfRadioNames);
        ui->cwMemoryRadioListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DeletedRadioForVoiceCwMemoryButtonsDialog::accepted);

    // Connect the "Cancel" button to close the dialog
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DeletedRadioForVoiceCwMemoryButtonsDialog::reject);

}

DeletedRadioForVoiceCwMemoryButtonsDialog::~DeletedRadioForVoiceCwMemoryButtonsDialog()
{
    delete ui;
}


QList<QListWidgetItem *> DeletedRadioForVoiceCwMemoryButtonsDialog::getSelectedItems(VoiceKeyerId id)
{

    QList<QListWidgetItem *> none;

    if (id == VoiceKeyerId::RigControl)
    {
        return ui->voiceMemoryRadioListWidget->selectedItems();
    }
    else if (id == VoiceKeyerId::CW_RigControl)
    {
        return ui->cwMemoryRadioListWidget->selectedItems();
    }

    return none;

}




void DeletedRadioForVoiceCwMemoryButtonsDialog::accepted()
{
   selectedItems = getSelectedItems(id);


   QString msgText;
   if (!selectedItems.isEmpty())
   {

       if (id ==VoiceKeyerId::RigControl)
       {
           msgText.append("Voice Memory Radios:-\n");
       }
       else if (id ==VoiceKeyerId::CW_RigControl)
       {
           msgText.append("CW Memory Radios:-\n");
       }

       foreach (QListWidgetItem* item, selectedItems)
       {
            msgText.append(item->text() + "\n");
       }

        msgText.append("\n");

   }

      if (!selectedItems.isEmpty())
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





