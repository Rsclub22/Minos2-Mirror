#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    virtual ~OptionsDialog() override;

private slots:
    void on_OKButton_clicked();

    void on_cancelButton_clicked();

    void doCloseEvent();

public Q_SLOTS:
    virtual int exec() override;
    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
