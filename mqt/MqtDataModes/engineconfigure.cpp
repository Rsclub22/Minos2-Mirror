#include <QSettings>
#include <QFileDialog>

#include "dmmainwindow.h"
#include "fileutils.h"
#include "engineconfigure.h"
#include "ui_engineconfigure.h"


EngineConfigure::EngineConfigure(DMMainWindow *parent) :
    QDialog(parent),
    ui(new Ui::EngineConfigure),
    mainWindow(parent)
{
    ui->setupUi(this);

    ui->MMVARIRX->addItems(mainWindow->inputDevices);
    ui->MMVARITX->addItems(mainWindow->outputDevices);

    QSettings settings;
    QString eStr = QString("dataModes/engines/");

    // MMVARI.ocx Has to be alongside the executable, BUT
    // we need to configure the MMVARI sound card!

    QString m = QCoreApplication::applicationDirPath() + "/MMVARI.ocx";
    ui->mmvariEdit->setText(m);

    m = settings.value(eStr + "MMVARI/input").toString();
    ui->MMVARIRX->setCurrentText(m);

    m = settings.value(eStr + "MMVARI/output").toString();
    ui->MMVARITX->setCurrentText(m);


    m = settings.value(eStr + "2Tone").toString();
    ui->twotoneEdit->setText(m);

    m = settings.value(eStr + "MMTTY").toString();
    ui->mmttyEdit->setText(m);

    m = settings.value(eStr + "FLDigi").toString();
    ui->fldigiEdit->setText(m);

    m = settings.value(eStr + "Gritty").toString();
    ui->grittyEdit->setText(m);
}

EngineConfigure::~EngineConfigure()
{
    delete ui;
}

void EngineConfigure::doBrowse(QString key, QLineEdit *edit)
{
    QSettings settings;
    QString eStr = QString("dataModes/engines/") + key;

    QString fname = settings.value(eStr).toString();

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
    doBrowse("MMTTY", ui->mmttyEdit);
}


void EngineConfigure::on_twotoneBrowse_clicked()
{
    doBrowse("2Tone", ui->twotoneEdit);
}


void EngineConfigure::on_fldigiBrowse_clicked()
{
    doBrowse("FLDigi", ui->fldigiEdit);
}

void EngineConfigure::on_grittyBrowse_clicked()
{
    doBrowse("Gritty", ui->grittyEdit);
}


void EngineConfigure::on_OKButton_clicked()
{
    QSettings settings;
    QString eStr = QString("dataModes/engines/");

    //settings.setValue(eStr + "MMVARI", ui->mmvariEdit->text());
    settings.setValue(eStr + "MMVARI/input", ui->MMVARIRX->currentText());
    settings.setValue(eStr + "MMVARI/output", ui->MMVARITX->currentText());

    settings.setValue(eStr + "MMTTY", ui->mmttyEdit->text());
    settings.setValue(eStr + "2Tone", ui->twotoneEdit->text());
    settings.setValue(eStr + "FLDigi", ui->fldigiEdit->text());
    settings.setValue(eStr + "Gritty", ui->grittyEdit->text());

    accept();
}


void EngineConfigure::on_cancelButton_clicked()
{
    reject();
}

