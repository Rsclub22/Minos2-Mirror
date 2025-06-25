#ifndef DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H
#define DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "txkeyerCommonConstants.h"

namespace Ui {
class DeletedRadioForVoiceCwMemoryButtonsDialog;
}

using namespace TxKeyerCommon;

class DeletedRadioForVoiceCwMemoryButtonsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeletedRadioForVoiceCwMemoryButtonsDialog(QStringList listOfRadioNames, TxKeyerId id, QWidget *parent = nullptr);
    ~DeletedRadioForVoiceCwMemoryButtonsDialog();

    QList<QListWidgetItem *> getSelectedItems(TxKeyerId id);


private slots:
    void accepted();

private:

    Ui::DeletedRadioForVoiceCwMemoryButtonsDialog *ui;


    TxKeyerId id = TxKeyerId::None;
    QList<QListWidgetItem*> selectedItems;


};

#endif // DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H
