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
    ~OptionsDialog();

public Q_SLOTS:
        virtual int exec() override;

private slots:
    void on_OKButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
