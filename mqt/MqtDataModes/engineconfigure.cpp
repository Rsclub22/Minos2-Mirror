#include <QSettings>
#include <QFileDialog>


#include "MinosRPC.h"
#include "dmmainwindow.h"
#include "fileutils.h"
#include "engineconfigure.h"
#include "ui_engineconfigure.h"

/*static*/ void EngineConfigure::setEnginePath(QString engine, QString path)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    QString eStr = QString("engines/");
    settings.setValue(eStr + engine, path);
}

/*static*/ QString EngineConfigure::getEnginePath(QString engine)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    QString eStr = QString("engines/");
    QString m = settings.value(eStr + engine).toString();

    return m;
}
/*static*/ void EngineConfigure::setAppPath(QString engine, QString path)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    QString appName = MinosRPC::getMinosRPC()->getAppName();
    settings.setValue(appName + "/" + engine, path);
}

/*static*/ QString EngineConfigure::getAppPath(QString engine)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    QString appName = MinosRPC::getMinosRPC()->getAppName();
    QString m = settings.value(appName + "/" + engine).toString();

    return m;
}

/*static*/ QString EngineConfigure::getAppCurrent()
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    QString appName = MinosRPC::getMinosRPC()->getAppName();
    QString m = settings.value(appName + "/current").toString();

    return m;
}

/*static*/ void EngineConfigure::setAppCurrent(QString engine)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    QString appName = MinosRPC::getMinosRPC()->getAppName();
    settings.setValue(appName + "/current", engine);
}

EngineConfigure::EngineConfigure(DMMainWindow *parent) :
    QDialog(parent),
    ui(new Ui::EngineConfigure),
    mainWindow(parent)
{
    ui->setupUi(this);

    ui->MMVARIRX->addItems(mainWindow->inputDevices);
    ui->MMVARITX->addItems(mainWindow->outputDevices);

    // MMVARI.ocx Has to be alongside the executable, BUT
    // we need to configure the MMVARI sound card!

    QString m = QCoreApplication::applicationDirPath() + "/MMVARI.ocx";
    ui->mmvariEdit->setText(m);
    m = getAppPath(DMMainWindow::mmvari + "/input");
    ui->MMVARIRX->setCurrentText(m);

    m = getAppPath(DMMainWindow::mmvari + "/output");
    ui->MMVARITX->setCurrentText(m);

    // Other engines have their own soundcard configuration

    m = getEnginePath(DMMainWindow::twotone);
    ui->twotoneEdit->setText(m);

    m = getEnginePath(DMMainWindow::mmtty);
    ui->mmttyEdit->setText(m);

    m = getEnginePath(DMMainWindow::fldigi);
    ui->fldigiEdit->setText(m);

    m = getEnginePath(DMMainWindow::gritty);
    ui->grittyEdit->setText(m);
}

EngineConfigure::~EngineConfigure()
{
    delete ui;
}
void EngineConfigure::doBrowse(QString key, QLineEdit *edit)
{
    QString fname = getEnginePath(key);

    QString InitialDir = GetCurrentDir();
    if (!fname.isEmpty())
    {
        InitialDir = ExtractFileDir(fname);
    }

    QString Filter = tr("Engine Files (*.ocx *.exe);;All Files (*.*)") ;


    QString fileName = QFileDialog::getOpenFileName( this,
                       tr("%1 Engine Executable").arg(key),
                       InitialDir,                   // opendir
                       Filter );

    if (!fileName.isEmpty())
    {
        edit->setText(fileName);
    }

}

void EngineConfigure::on_mmttyBrowse_clicked()
{
    doBrowse(DMMainWindow::mmtty, ui->mmttyEdit);
}


void EngineConfigure::on_twotoneBrowse_clicked()
{
    doBrowse(DMMainWindow::twotone, ui->twotoneEdit);
}


void EngineConfigure::on_fldigiBrowse_clicked()
{
    doBrowse(DMMainWindow::fldigi, ui->fldigiEdit);
}

void EngineConfigure::on_grittyBrowse_clicked()
{
    doBrowse(DMMainWindow::gritty, ui->grittyEdit);
}


void EngineConfigure::on_OKButton_clicked()
{
    //setEnginePath(DMMainWindow::mmvari, ui->mmvariEdit->text());
    setAppPath(DMMainWindow::mmvari + "/input", ui->MMVARIRX->currentText());
    setAppPath(DMMainWindow::mmvari + "/output", ui->MMVARITX->currentText());

    setEnginePath(DMMainWindow::mmtty, ui->mmttyEdit->text());
    setEnginePath(DMMainWindow::twotone, ui->twotoneEdit->text());
    setEnginePath(DMMainWindow::fldigi, ui->fldigiEdit->text());
    setEnginePath(DMMainWindow::gritty, ui->grittyEdit->text());

    accept();
}


void EngineConfigure::on_cancelButton_clicked()
{
    reject();
}

