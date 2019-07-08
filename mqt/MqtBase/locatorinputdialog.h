#ifndef LOCATORINPUTDIALOG_H
#define LOCATORINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class LocatorInputDialog;
}

class LocatorInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LocatorInputDialog(QWidget *parent, QString textToEdit, const QString dialogTitle, const QString lineEditLabel);
    ~LocatorInputDialog();

    QString getText();

private:
    Ui::LocatorInputDialog *ui;
};

#endif // LOCATORINPUTDIALOG_H
