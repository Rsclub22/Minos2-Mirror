#ifndef DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H
#define DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "voicekeyerCommonConstants.h"

namespace Ui {
class DeletedRadioForVoiceCwMemoryButtonsDialog;
}

using namespace voiceKeyerCommon;

class DeletedRadioForVoiceCwMemoryButtonsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeletedRadioForVoiceCwMemoryButtonsDialog(QStringList listOfRadioNames, VoiceKeyerId id, QWidget *parent = nullptr);
    ~DeletedRadioForVoiceCwMemoryButtonsDialog();

    QList<QListWidgetItem *> getSelectedItems(VoiceKeyerId id);


private slots:
    void accepted();

private:

    Ui::DeletedRadioForVoiceCwMemoryButtonsDialog *ui;


    VoiceKeyerId id = VoiceKeyerId::None;
    QList<QListWidgetItem*> selectedItems;


};

#endif // DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H
