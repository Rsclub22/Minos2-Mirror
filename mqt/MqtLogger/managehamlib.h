#ifndef MANAGEHAMLIB_H
#define MANAGEHAMLIB_H

#include <QDialog>

namespace Ui {
class ManageHamlib;
}

class ManageHamlib : public QDialog
{
    Q_OBJECT

public:
    explicit ManageHamlib(QWidget *parent = nullptr);
    ~ManageHamlib();

private slots:
    void on_cancelButton_clicked();
private:
    Ui::ManageHamlib *ui;
};

#endif // MANAGEHAMLIB_H
