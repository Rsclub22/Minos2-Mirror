#include "userclustercommanddialog.h"
#include "ui_userclustercommanddialog.h"


ClusterUserCommandData::ClusterUserCommandData(QString _name, QString _cmdString)
{
    name = _name;
    cmdString = _cmdString;
}


ClusterUserCommandData::ClusterUserCommandData()
{

}




userClusterCommandDialog::userClusterCommandDialog(QWidget *parent, int buttonNumber, ClusterUserCommandData* _editData, ClusterUserCommandData* _curData, QString name) :
    QDialog(parent),
    ui(new Ui::userClusterCommandDialog)
    ,editData(nullptr)
    ,curData(nullptr)
    ,nameChanged(false)
    ,cmdStringChanged(false)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    editData = _editData;
    curData = _curData;
    setWindowTitle(QString("Cluster User Command %1 - %2").arg(QString::number(buttonNumber + 1)).arg(name));
    ui->name->setText(curData->name);
    ui->commandString->setText(curData->cmdString);

    connect(ui->name, SIGNAL(editingFinished()), this, SLOT(nameEditFinished()));
    connect(ui->commandString, SIGNAL(editingFinished()), this, SLOT(cmdStringEditFinished()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(editAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(editRejected()));


}




userClusterCommandDialog::~userClusterCommandDialog()
{
    delete ui;
}

void userClusterCommandDialog::nameEditFinished()
{
    QString n = ui->name->text().trimmed();
    if (n != curData->name)
    {
        editData->name = n;
        nameChanged = true;
    }
}

void userClusterCommandDialog::cmdStringEditFinished()
{
    QString c = ui->commandString->text().trimmed();
    if (c != curData->cmdString)
    {
        editData->cmdString = c;
        cmdStringChanged = true;
    }
}


void userClusterCommandDialog::editAccepted()
{
    return;
}

void userClusterCommandDialog::editRejected()
{

    if (nameChanged)
    {
        editData->name = curData->name;
        nameChanged = false;
    }
    if (cmdStringChanged)
    {
        editData->cmdString = curData->cmdString;
        cmdStringChanged = false;
    }

}
