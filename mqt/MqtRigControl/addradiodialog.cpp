/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018 - 2021
//
//
/////////////////////////////////////////////////////////////////////////////
#include "addradiodialog.h"
#include "ui_addradiodialog.h"

#include <QMessageBox>

AddRadioDialog::AddRadioDialog(QMap<QString, QSharedPointer<scatParams> > *availRadioData_, RigFactory* rigFactory, QString windowTitle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddRadioDialog)
{
    ui->setupUi(this);
    availRadioData = availRadioData_;

    setWindowTitle(windowTitle);

    errorDialogTitle = "Error - " + windowTitle;

    rigFactory->populateComboRigList(ui->radioModel);
    radioModel = ui->radioModel->currentText();
    connect (ui->radioModel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddRadioDialog::radioModelSelect);


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
            msgBox.setWindowTitle(errorDialogTitle);
            msgBox.setText(tr("Radio Name Empty\nPlease enter a name for the radio"));
            msgBox.exec();
            ui->radioName->setFocus();
            return;
        }
        else if (containsChars(ui->radioName->text(), illegalChars))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle(errorDialogTitle);
            msgBox.setModal( true );
            msgBox.setText(tr("Radio name contains invalid characters,\n please remove non-alpha or non-numeric characters"));
            msgBox.exec();
            return;
        }
        else if (checkNameAlreadyExists(ui->radioName->text()))
        {

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


void AddRadioDialog::hideRadioSelection(bool state)
{
    ui->radioModel->setVisible(state);
    ui->radioModelLbl->setVisible(state);
}

bool AddRadioDialog::checkNameAlreadyExists(QString radioName)
{
    QStringList rList = availRadioData->keys();
    QMessageBox msgBox;
    msgBox.setModal( true );
    msgBox.setWindowTitle(tr("Error Radio Name"));

    foreach (const auto &r, rList)
    {
        if (availRadioData->value(r)->radioName == radioName && availRadioData->value(r)->markForDeletion)
        {

            msgBox.setText(tr("Radio %1 is marked for deletion,\n please use another name").arg(radioName));
            msgBox.exec();
            return true;
        }
        else if (availRadioData->value(r)->radioName == radioName && !availRadioData->value(r)->markForDeletion)
        {
            msgBox.setText(tr("Radio %1 already exists,\n please use another name").arg(radioName));
            msgBox.exec();
            return true;
        }
    }

    return false;
}




QString AddRadioDialog::getRadioName()
{
    return radioName.trimmed();
}

QString AddRadioDialog::getRadioModel()
{
    return radioModel;
}
