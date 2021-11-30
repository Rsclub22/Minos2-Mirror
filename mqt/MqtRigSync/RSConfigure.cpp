#include <QSettings>
#include "MShowMessageDlg.h"
#include "cutils.h"
#include "RSConfigure.h"
#include "ui_RSConfigure.h"

RSConfigure::RSConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RSConfigure)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QSettings settings;
    QByteArray geometry = settings.value("RSConfigure/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RIGSYNC_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);

    wsjtxPort = config.value("WSJT-X port", 2237).toInt();
    wsjtxAddress = config.value( "WSJT-X address", "" ).toString();

    ui->wsjtxPort->setValue(wsjtxPort);
    ui->wsjtxAddress->setText(wsjtxAddress);

    n1mmPort = config.value("N1MM+ port", 12060).toInt();

    ui->n1mmPort->setValue(n1mmPort);
}

RSConfigure::~RSConfigure()
{
    delete ui;
}

void RSConfigure::setServerList(QStringList rigServers, QString mainCurrent, QString subCurrent)
{
    ui->mainControl->clear();
    ui->mainControl->addItems(rigServers);
    ui->mainControl->setCurrentText(mainCurrent);

    ui->subControl->clear();
    ui->subControl->addItems(rigServers);
    ui->subControl->setCurrentText(subCurrent);
}

QString RSConfigure::getMainServer()
{
    return ui->mainControl->currentText();
}
QString RSConfigure::getSubServer()
{
    return ui->subControl->currentText();
}
void RSConfigure::on_OKButton_clicked()
{
    if ( !getMainServer().isEmpty() && getMainServer() == getSubServer())
    {
        mShowMessage("Main and Sub rig cannot be on the same RigControl Instance", this);
    }
    else
    {
        wsjtxPort = ui->wsjtxPort->value();
        wsjtxAddress = ui->wsjtxAddress->text().trimmed();
        n1mmPort = ui->n1mmPort->value();

        QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RIGSYNC_CONFIG_FILE;
        QSettings config(fileName, QSettings::IniFormat);

        config.setValue("WSJT-X port", wsjtxPort);
        config.setValue( "WSJT-X address", wsjtxAddress );

        config.setValue("N1MM+ port", n1mmPort);

        accept();
    }
}

void RSConfigure::on_canceButton_clicked()
{
    reject();
}
void RSConfigure::doCloseEvent()
{
    QSettings settings;
    settings.setValue("RSConfigure/geometry", saveGeometry());
}
void RSConfigure::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void RSConfigure::accept()
{
    doCloseEvent();
    QDialog::accept();
}
