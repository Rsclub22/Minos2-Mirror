#ifndef ADDRADIODIALOG_H
#define ADDRADIODIALOG_H

#include <QDialog>
#include <QMap>
#include "rigfactory.h"
#include "cutils.h"

namespace Ui {
class AddRadioDialog;
}

class AddRadioDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddRadioDialog(QMap<QString, QSharedPointer<scatParams> > *availRadioData_, RigFactory* rigFactory, QString windowTitle, QWidget *parent = nullptr);
    ~AddRadioDialog();

    QString getRadioName();
    QString getRadioModel();


    void hideRadioSelection(bool state);
private slots:

    void radioModelSelect(int index);


private:
    Ui::AddRadioDialog *ui;

    QMap<QString, QSharedPointer<scatParams> > *availRadioData;
    QString radioName;
    QString radioModel;

    QString errorDialogTitle;

    void done(int r);   // override done function to validate data entry
    bool checkNameAlreadyExists(QString radioName);
};

#endif // ADDRADIODIALOG_H
