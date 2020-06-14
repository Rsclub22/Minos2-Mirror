#include "addantennadialog.h"
#include "ui_addantennadialog.h"
#include <QMessageBox>

AddAntennaDialog::AddAntennaDialog(QStringList _availAntennas, RotatorFactory *rotatorFactory, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAntennaDialog)
{
    ui->setupUi(this);
    availAntennas = _availAntennas;

    ui->rotatorModel->addItem("");   // add blank at begining
    for (auto r = rotatorFactory->supported_rotators()->cbegin(); r != rotatorFactory->supported_rotators()->cend(); ++r)
    {
        QString rotText = r.key();
        ui->rotatorModel->addItem(rotText);
    }
    rotatorModel = ui->rotatorModel->currentText();
    connect (ui->rotatorModel, SIGNAL(currentIndexChanged(int)), this, SLOT(rotatorModelSelect(int)));

}

AddAntennaDialog::~AddAntennaDialog()
{
    delete ui;
}






void AddAntennaDialog::rotatorModelSelect(int /*index*/)
{

    rotatorModel = ui->rotatorModel->currentText();

}


// override done function to validate data entry

void AddAntennaDialog::done(int r)
{
    if(QDialog::Accepted == r)  // ok was pressed
    {
        if (ui->antennaName->text() == "")
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Antenna Name Empty\nPlease enter a name for the antenna"));
            msgBox.exec();
            ui->antennaName->setFocus();
            return;
        }
        else if (containsChars(ui->antennaName->text(), illegalChars))
        {
            QMessageBox msgBox;
            msgBox.setModal( true );
            msgBox.setText(tr("Antenna name contains invalid characters,\n please remove non-alpha or non-numeric characters"));
            msgBox.exec();
            return;
        }
        else if (availAntennas.contains(ui->antennaName->text()))
        {
            QMessageBox msgBox;
            msgBox.setModal( true );
            msgBox.setText(tr("Antenna name already exists,\n please use another name"));
            msgBox.exec();
            return;
        }
        else
        {
            antennaName = ui->antennaName->text().trimmed();
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


QString AddAntennaDialog::getAntennaName()
{
    return antennaName;
}

QString AddAntennaDialog::getRotatorModel()
{
    return rotatorModel;
}

