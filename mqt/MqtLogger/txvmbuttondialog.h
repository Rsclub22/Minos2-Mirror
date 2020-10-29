#ifndef TXVMBUTTONDIALOG_H
#define TXVMBUTTONDIALOG_H

#include <QDialog>

namespace Ui {
class TxVmButtonDialog;
}

class TxVmButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmButtonDialog(QWidget *parent = nullptr);
    ~TxVmButtonDialog();

private:
    Ui::TxVmButtonDialog *ui;
};

#endif // TXVMBUTTONDIALOG_H
