#ifndef CALLSIGNINPUTDIALOG_H
#define CALLSIGNINPUTDIALOG_H

#include <QDialog>
#include "cutils.h"

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

    void setText(QString text);
    bool isValid();


private:
    Ui::CallsignInputDialog *ui;
    UpperCaseValidator ucValidator;

};

#endif // CALLSIGNINPUTDIALOG_H
