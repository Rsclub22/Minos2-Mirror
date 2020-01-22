/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
//
/////////////////////////////////////////////////////////////////////////////



#include "logdialog.h"
#include "ui_logdialog.h"
#include <QFile>
#include <QDateTime>
#include <QPushButton>
#include <QFileDialog>
#include <QSpinBox>
#include <QSettings>
#include <QKeyEvent>

LogDialog::LogDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->bearingDifferenceSBox->setRange(1,20);
    readLogConfig();
}

LogDialog::~LogDialog()
{
    delete ui;
}





void LogDialog::on_logDirBrowsePb_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Select Bearing Log Folder"), "./Configuration/BearingLog");
    if (dir != directory)
    {
        directory = dir;
        dirChanged = true;
    }
}






void LogDialog::on_logFilenameEdit_textChanged(const QString &arg1)
{
    QString name = arg1;
    if (name != filename)
    {
        filename = name;
        filenameChanged = true;
    }
}



void LogDialog:: saveLogConfig()
{

    bool changed = false;
    QSettings config("./Configuration/MinosRotatorConfig.ini", QSettings::IniFormat);
    config.beginGroup("BearingLog");
    if (dirChanged)
    {
        config.setValue("directory", directory);
        dirChanged = false;
        changed = true;
    }
    if (filenameChanged)
    {
        config.setValue("filename", filename);
        filenameChanged = false;
        changed = true;
    }
    if (bearDiffChanged)
    {
        config.setValue("bearing_difference", bearingDiff);
        bearDiffChanged = false;
        changed = true;
    }
    if (logEnabledChanged)
    {
        config.setValue("loggingEnabled", logEnabled);
        logEnabledChanged = false;
        changed = true;
    }


    config.endGroup();
    if (changed)
    {
        emit bearingLogConfigChanged();
    }

}




void LogDialog::readLogConfig()
{
    QSettings config("./Configuration/MinosRotatorConfig.ini", QSettings::IniFormat);
    config.beginGroup("BearingLog");

    directory = config.value("directory", "./BearingLog").toString();
    filename = config.value("filename", "bearingLog").toString();
    bearingDiff = config.value("bearing_difference", 0).toInt();
    logEnabled = config.value("loggingEnabled", false).toBool();

    config.endGroup();
}


void LogDialog::loadLogConfig()
{
    ui->logDirectoryEdit->setText(directory);
    ui->logFilenameEdit->setText(filename);
    ui->bearingDifferenceSBox->setValue(bearingDiff);
    ui->logcheckBox->setChecked(logEnabled);
    emit showLogDialog();
}


void LogDialog::keyPressEvent(QKeyEvent *event)
{

    switch (event->key())
    {
        case Qt::Key_Escape:
            close();
        break;

    }
}

void LogDialog::on_logDialogOkPb_accepted()
{
    saveLogConfig();
}

void LogDialog::on_logDialogOkPb_rejected()
{
    close();
}

void LogDialog::on_bearingDifferenceSBox_valueChanged(int arg1)
{
    if (arg1 != bearingDiff)
    {
        bearingDiff = arg1;
        bearDiffChanged = true;
    }
}

void LogDialog::on_logcheckBox_toggled(bool checked)
{
    if (checked != logEnabled)
    {
        logEnabled = checked;
        logEnabledChanged = true;
    }
}
