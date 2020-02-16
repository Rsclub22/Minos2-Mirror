#ifndef ADDRADIODIALOG_H
#define ADDRADIODIALOG_H

#include <QDialog>
#include "rigfactory.h"
#include "cutils.h"

namespace Ui {
class AddRadioDialog;
}

class AddRadioDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddRadioDialog(QStringList _availRadios, RigFactory* rigFactory, QWidget *parent = nullptr);
    ~AddRadioDialog();

    QString getRadioName();
    QString getRadioModel();


private slots:

    void radioModelSelect(int index);


private:
    Ui::AddRadioDialog *ui;
    QStringList availRadios;
    QString radioName;
    QString radioModel;

    void done(int r);   // override done function to validate data entry
};

#endif // ADDRADIODIALOG_H
