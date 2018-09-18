#ifndef ADDANTENNADIALOG_H
#define ADDANTENNADIALOG_H

#include <QDialog>
#include "rotcontrol.h"
#include "cutils.h"

namespace Ui {
class AddAntennaDialog;
}

class AddAntennaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAntennaDialog(QStringList _availAntennas, RotControl* rot, QWidget *parent = nullptr);
    ~AddAntennaDialog();

    QString getAntennaName();
    QString getRotatorModel();

private slots:

    void rotatorModelSelect(int index);


private:
    Ui::AddAntennaDialog *ui;
    QStringList availAntennas;
    QString antennaName;
    QString rotatorModel;

    void done(int r);   // override done function to validate data entry
};

#endif // ADDANTENNADIALOG_H
