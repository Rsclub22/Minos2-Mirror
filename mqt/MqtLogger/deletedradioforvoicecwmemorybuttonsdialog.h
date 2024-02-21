#ifndef DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H
#define DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class DeletedRadioForVoiceCwMemoryButtonsDialog;
}

class DeletedRadioForVoiceCwMemoryButtonsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeletedRadioForVoiceCwMemoryButtonsDialog(QStringList listOfRadioNames, QStringList listCwRadioNames, QWidget *parent = nullptr);
    ~DeletedRadioForVoiceCwMemoryButtonsDialog();

    QList<QListWidgetItem *> getSelectedItems();
private slots:
    void accepted();

private:

    Ui::DeletedRadioForVoiceCwMemoryButtonsDialog *ui;

    QList<QListWidgetItem *> getVoiceSelectedItems();
    QList<QListWidgetItem *> getCwSelectedItems();


};

#endif // DELETEDRADIOFORVOICECWMEMORYBUTTONSDIALOG_H
