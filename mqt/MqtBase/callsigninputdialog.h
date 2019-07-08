#ifndef CALLSIGNINPUTDIALOG_H
#define CALLSIGNINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class CallsignInputDialog;
}

class CallsignInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CallsignInputDialog(QWidget *parent, QString textToEdit, const QString dialogTitle, const QString lineEditLabel);
    ~CallsignInputDialog();

    QString getText();

private:
    Ui::CallsignInputDialog *ui;
};

#endif // CALLSIGNINPUTDIALOG_H
