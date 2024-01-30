#ifndef MANAGEADIFDIALOG_H
#define MANAGEADIFDIALOG_H

#include <QDialog>

namespace Ui {
class ManageAdifDialog;
}

class ManageAdifDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManageAdifDialog(QWidget *parent = nullptr);
    ~ManageAdifDialog();

private slots:
    void on_OKButton_clicked();

    void on_cancelButton_clicked();

    void on_fileNameBrowse_clicked();

    void on_monitorButton_clicked();

    void on_appendButton_clicked();

private:
    Ui::ManageAdifDialog *ui;
};

#endif // MANAGEADIFDIALOG_H
