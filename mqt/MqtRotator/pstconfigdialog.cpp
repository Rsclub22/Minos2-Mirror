#include "rotatorcommon.h"

#include "pstconfigdialog.h"
#include "ui_pstconfigdialog.h"

PstConfigDialog::PstConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PstConfigDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    loadDialog();


}

PstConfigDialog::~PstConfigDialog()
{
    delete ui;
}

void PstConfigDialog::loadDialog()
{
    QString fileName = PST_CONFIG_FILE();
    QSettings  config(fileName, QSettings::IniFormat);

    config.beginGroup("PSTRotatorPath");

    pstRotatorFilePathx86 = config.value("pstRotatorPathx86", "C:/Program Files (x86)/PstRotator/").toString();
    pstRotatorFilePath = config.value("pstRotatorPath", "C:/Program Files/PstRotator/").toString();

    pstRotatorAzFilePathx86 = config.value("pstRotatorAzPathx86", "C:/Program Files (x86)/PstRotatorAz/").toString();
    pstRotatorAzFilePath = config.value("pstRotatorAzPath", "C:/Program Files/PstRotatorAz/").toString();


    config.endGroup();

    ui->pstRotatorPathx86LineEdit->setText(pstRotatorFilePathx86);
    ui->pstRotatorPathLineEdit->setText(pstRotatorFilePath);
    ui->pstRotatorAzPathx86LineEdit->setText(pstRotatorAzFilePathx86);
    ui->pstRotatorAzPathLineEdit->setText(pstRotatorAzFilePath);

}

QString PstConfigDialog::getPstRotatorFilePathx86Text()
{
    return ui->pstRotatorPathx86LineEdit->text().trimmed();
}

QString PstConfigDialog::getpstRotatorFilePathText()
{
    return ui->pstRotatorPathLineEdit->text().trimmed();
}

QString PstConfigDialog::getPstRotatorAzFilePathx86Text()
{
    return ui->pstRotatorAzPathx86LineEdit->text().trimmed();
}

QString PstConfigDialog::getPstRotatorAzFilePathText()
{
    return ui->pstRotatorAzPathLineEdit->text().trimmed();
}

