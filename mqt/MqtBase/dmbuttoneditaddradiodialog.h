#ifndef DMBUTTONEDITADDRADIODIALOG_H
#define DMBUTTONEDITADDRADIODIALOG_H

#include <QDialog>

namespace Ui {
class DmButtonEditAddRadioDialog;
}

class DmButtonEditAddRadioDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DmButtonEditAddRadioDialog(const QStringList listofRadios, QWidget *parent = nullptr);
    ~DmButtonEditAddRadioDialog();


    QString getRadioName();

private slots:
    void radioNameSelect();
private:
    Ui::DmButtonEditAddRadioDialog *ui;
    QString radioName;

};

#endif // DMBUTTONEDITADDRADIODIALOG_H
