#include "addradiodialog.h"
#include "ui_addradiodialog.h"

#include <QMessageBox>

AddRadioDialog::AddRadioDialog(QStringList _availRadios, RigControl* rig, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddRadioDialog)
{
    ui->setupUi(this);
    availRadios = _availRadios;



    rig->getRigList(ui->radioModel);
    radioModel = ui->radioModel->currentText();
    connect (ui->radioModel, SIGNAL(currentIndexChanged(int)), this, SLOT(radioModelSelect(int)));


}

AddRadioDialog::~AddRadioDialog()
{
    delete ui;
}





void AddRadioDialog::radioModelSelect(int /*index*/)
{

    radioModel = ui->radioModel->currentText();

}




// override done function to validate data entry

void AddRadioDialog::done(int r)
{
    if(QDialog::Accepted == r)  // ok was pressed
    {
        if (ui->radioName->text() == "")
        {
            QMessageBox msgBox;
            msgBox.setText("Radio Name Empty\nPlease enter a name for the radio");
            msgBox.exec();
            ui->radioName->setFocus();
            return;
        }
        else if (containsChars(ui->radioName->text(), illegalChars))
        {
            QMessageBox msgBox;
            msgBox.setModal( true );
            msgBox.setText("Radio name contains invalid characters,\n please remove non-alpha or non-numeric characters");
            msgBox.exec();
            return;
        }
        else if (availRadios.contains(ui->radioName->text()))
        {
            QMessageBox msgBox;
            msgBox.setModal( true );
            msgBox.setText("Radio name already exists,\n please use another name");
            msgBox.exec();
            return;
        }
        else
        {
            radioName = ui->radioName->text().trimmed();
            QDialog::done(r);
            return;
        }

    }
    else    // cancel, close or exc was pressed
    {
        QDialog::done(r);
        return;
    }
}






QString AddRadioDialog::getRadioName()
{
    return radioName;
}

QString AddRadioDialog::getRadioModel()
{
    return radioModel;
}
