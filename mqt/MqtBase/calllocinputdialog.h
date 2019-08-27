#ifndef CALLLOCINPUTDIALOG_H
#define CALLLOCINPUTDIALOG_H

#include <QDialog>
#include "cutils.h"

namespace Ui {
class CallLocInputDialog;
}

class CallLocInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CallLocInputDialog(QWidget *parent, QString textToEdit, const QString dialogTitle, const QString lineEditLable);
    ~CallLocInputDialog();


    QString getText();
private:
    Ui::CallLocInputDialog *ui;
    UpperCaseValidator ucValidator;


};

#endif // CALLLOCINPUTDIALOG_H
