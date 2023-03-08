#include <QSettings>
#include <QFileDialog>

#include "dmmainwindow.h"
#include "enginewindow.h"
#include "fileutils.h"

#include "engineconfigure.h"
#include "ui_engineconfigure.h"

void EngineConfigure::setEnginePath(QSettings &settings, QString engine, QString path)
{
    QString eStr = QString("engines/");
    settings.setValue(eStr + engine, path);
}
/*static*/ void EngineConfigure::setEnginePath(QString engine, QString path)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    setEnginePath(settings, engine, path);
}

/*static*/ QString EngineConfigure::getEnginePath(QSettings &settings, QString engine)
{
    QString eStr = QString("engines/");
    QString m = settings.value(eStr + engine).toString();

    return m;
}
/*static*/ QString EngineConfigure::getEnginePath(QString engine)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    return getEnginePath(settings, engine);
}


void EngineConfigure::setSpeed(QSettings &settings, QString mode, QString speed)
{
    settings.setValue( mode, speed);
}


void EngineConfigure::setSpeed(QString mode, QString speed)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    setSpeed(settings, mode, speed);
}

QString EngineConfigure::getSpeed(QSettings &settings, QString mode)
{
    QString s = settings.value( mode, 0).toString();

    return s;
}
QString EngineConfigure::getSpeed(QString mode)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    return getSpeed(settings, mode);
}

bool EngineConfigure::getEngineEnabled(QSettings &settings, QString engine)
{
    bool s = settings.value("enabled/" + engine, false).toBool();
    return s;
}

bool EngineConfigure::getEngineEnabled(QString engine)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    return getEngineEnabled(settings, engine);
}

void EngineConfigure::setEngineEnabled(QSettings &settings, QString engine, bool enabled)
{
    settings.setValue("enabled/" + engine, enabled);
}
void EngineConfigure::setEngineEnabled(QString engine, bool enabled)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    setEngineEnabled(settings, engine, enabled);
}

QString EngineConfigure::getEngineSound(QSettings &settings, QString engine, QString io)
{
    QString s = settings.value("sound/" + engine + "/" + io).toString();
    return s;
}

QString EngineConfigure::getEngineSound(QString engine, QString io)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    return getEngineSound(settings, engine, io);
}

void EngineConfigure::setEngineSound(QSettings &settings, QString engine, QString io, QString s)
{
    settings.setValue("sound/" + engine + "/" + io, s);
}

void EngineConfigure::setEngineSound(QString engine, QString io, QString s)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    setEngineSound(settings, engine, io, s);
}

int EngineConfigure::getEnginePort(QSettings &settings, QString engine)
{
    int p = settings.value("port/" + engine).toInt();
    return p;
}

int EngineConfigure::getEnginePort(QString engine)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    return getEnginePort(settings, engine);
}

void EngineConfigure::setEnginePort(QSettings &settings, QString engine, int port)
{
    settings.setValue("port/" + engine, port);
}

void EngineConfigure::setEnginePort(QString engine, int port)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    setEnginePort(settings, engine, port);
}
bool EngineConfigure::check()
{
    // check

    // all paths exist

    // nothing enabled for which there is no path

    // ports are set where required

    return true;
}
EngineConfigure::EngineConfigure(DMMainWindow *parent) :
    QDialog(parent),
    ui(new Ui::EngineConfigure),
    mainWindow(parent)
{
    ui->setupUi(this);

    {
        QSettings settings;
        geoStr = QString("dataModes/Configuration/geometry");
        QByteArray geometry = settings.value(geoStr).toByteArray();
        if (geometry.size() > 0)
            restoreGeometry(geometry);
    }
    ui->MMVARIRX1->addItems(mainWindow->inputDevices);
    ui->MMVARITX1->addItems(mainWindow->outputDevices);
    ui->MMVARIRX2->addItems(mainWindow->inputDevices);
    ui->MMVARITX2->addItems(mainWindow->outputDevices);

    // MMVARI.ocx Has to be alongside the executable, BUT
    // we need to configure the MMVARI sound card!

    QString m = QCoreApplication::applicationDirPath() + "/MMVARI.ocx";
    ui->mmvariEdit->setText(m);

    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);

    m = getEngineSound(settings, EngineWindow::mmvari + EngineWindow::i1, "input");
    ui->MMVARIRX1->setCurrentText(m);
    m = getEngineSound(settings, EngineWindow::mmvari + EngineWindow::i2, "input");
    ui->MMVARIRX2->setCurrentText(m);

    m = getEngineSound(settings, EngineWindow::mmvari + EngineWindow::i1, "output");
    ui->MMVARITX1->setCurrentText(m);
    m = getEngineSound(settings, EngineWindow::mmvari + EngineWindow::i2, "output");
    ui->MMVARITX2->setCurrentText(m);

    bool b;
    b = getEngineEnabled(settings, EngineWindow::mmvari + EngineWindow::i1);
    ui->mmvariEnable1->setChecked(b);
    b = getEngineEnabled(settings, EngineWindow::mmvari + EngineWindow::i2);
    ui->mmvariEnable2->setChecked(b);

    // Other engines have their own soundcard configuration

    m = getEnginePath(settings, EngineWindow::twotone + EngineWindow::i1);
    ui->twotoneEdit1->setText(m);
    m = getEnginePath(settings, EngineWindow::twotone + EngineWindow::i2);
    ui->twotoneEdit2->setText(m);

    b = getEngineEnabled(settings, EngineWindow::twotone + EngineWindow::i1);
    ui->twotoneEnable1->setChecked(b);
    b = getEngineEnabled(settings, EngineWindow::twotone + EngineWindow::i2);
    ui->twotoneEnable2->setChecked(b);

    m = getEnginePath(settings, EngineWindow::mmtty + EngineWindow::i1);
    ui->mmttyEdit1->setText(m);
    m = getEnginePath(settings, EngineWindow::mmtty + EngineWindow::i2);
    ui->mmttyEdit2->setText(m);

    b = getEngineEnabled(settings, EngineWindow::mmtty + EngineWindow::i1);
    ui->mmttyEnable1->setChecked(b);
    b = getEngineEnabled(settings, EngineWindow::mmtty + EngineWindow::i2);
    ui->mmttyEnable2->setChecked(b);

    m = getEnginePath(settings, EngineWindow::fldigi + EngineWindow::i1);
    ui->fldigiEdit->setText(m);

    b = getEngineEnabled(settings, EngineWindow::fldigi + EngineWindow::i1);
    ui->fldigiEnable1->setChecked(b);
    b = getEngineEnabled(settings, EngineWindow::fldigi + EngineWindow::i2);
    ui->fldigiEnable2->setChecked(b);

    m = getEnginePath(settings, EngineWindow::gritty + EngineWindow::i1);
    ui->grittyEdit->setText(m);

    b = getEngineEnabled(settings, EngineWindow::gritty + EngineWindow::i1);
    ui->grittyEnable1->setChecked(b);
    b = getEngineEnabled(settings, EngineWindow::gritty + EngineWindow::i2);
    ui->grittyEnable2->setChecked(b);

    b = getEngineEnabled(settings, EngineWindow::test + EngineWindow::i1);
    ui->testEnable1->setChecked(b);
    b = getEngineEnabled(settings, EngineWindow::test + EngineWindow::i2);
    ui->testEnable2->setChecked(b);

    int p = getEnginePort(settings, EngineWindow::fldigi + EngineWindow::i1);
    if (p == 0)
    {
        p = 7362;
    }
    ui->fldigiPort1->setText(QString::number(p));
    ui->fldigiPort1->setValidator(new QIntValidator(0, 0xffff, this));

    p = getEnginePort(settings, EngineWindow::fldigi + EngineWindow::i2);
    if (p == 0)
    {
        p = 7363;
    }
    ui->fldigiPort2->setText(QString::number(p));
    ui->fldigiPort2->setValidator(new QIntValidator(0, 0xffff, this));

    p = getEnginePort(settings, EngineWindow::gritty + EngineWindow::i1);
    if (p == 0)
    {
        p = 7502;
    }
    ui->grittyPort1->setText(QString::number(p));
    ui->grittyPort1->setValidator(new QIntValidator(0, 0xffff, this));

    p = getEnginePort(settings, EngineWindow::gritty + EngineWindow::i2);
    if (p == 0)
    {
        p = 7503;
    }
    ui->grittyPort2->setText(QString::number(p));
    ui->grittyPort2->setValidator(new QIntValidator(0, 0xffff, this));

    ui->rttySpeed->addItem("45.45");
    ui->rttySpeed->addItem("75");
    ui->rttySpeed->setCurrentText(getSpeed("RTTY"));

    ui->BPSKSpeed->addItem("31.25");
    ui->BPSKSpeed->addItem("62.5");
    ui->BPSKSpeed->setCurrentText(getSpeed("BPSK"));

    m = settings.value("Sender").toString();
    ui->senderCombo->addItem(QString());
    ui->senderCombo->addItems(EngineWindow::enginesList);
    ui->senderCombo->setCurrentText(m);
}

EngineConfigure::~EngineConfigure()
{
    delete ui;
}
void EngineConfigure::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    QWidget::closeEvent(event);
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

void EngineConfigure::on_mmttyBrowse1_clicked()
{
    doBrowse(EngineWindow::mmtty + EngineWindow::i1, ui->mmttyEdit1);
}

void EngineConfigure::on_mmttyBrowse2_clicked()
{
    doBrowse(EngineWindow::mmtty + EngineWindow::i2, ui->mmttyEdit2);
}


void EngineConfigure::on_twotoneBrowse1_clicked()
{
    doBrowse(EngineWindow::twotone + EngineWindow::i1, ui->twotoneEdit1);
}

void EngineConfigure::on_twotoneBrowse2_clicked()
{
    doBrowse(EngineWindow::twotone + EngineWindow::i2, ui->twotoneEdit1);
}


void EngineConfigure::on_fldigiBrowse_clicked()
{
    doBrowse(EngineWindow::fldigi + EngineWindow::i1, ui->fldigiEdit);
}

void EngineConfigure::on_grittyBrowse_clicked()
{
    doBrowse(EngineWindow::gritty + EngineWindow::i1, ui->grittyEdit);
}


void EngineConfigure::on_OKButton_clicked()
{
    // We need to do some cross checks -
    // Enable not checked when path doesn't exist (? disable until path exists) or no port specified
    // MMTTY/2Tone - paths are different
    // gritty/fldigi - ports are different
    //setEnginePath(EngineWindow::mmvari, ui->mmvariEdit->text());
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);

    setEngineSound(settings, EngineWindow::mmvari + EngineWindow::i1, "input", ui->MMVARIRX1->currentText());
    setEngineSound(settings, EngineWindow::mmvari + EngineWindow::i1, "output", ui->MMVARITX1->currentText());

    setEnginePath(settings, EngineWindow::mmtty + EngineWindow::i1, ui->mmttyEdit1->text());
    setEnginePath(settings, EngineWindow::mmtty + EngineWindow::i2, ui->mmttyEdit2->text());
    setEnginePath(settings, EngineWindow::twotone + EngineWindow::i1, ui->twotoneEdit1->text());
    setEnginePath(settings, EngineWindow::twotone + EngineWindow::i2, ui->twotoneEdit2->text());
    setEnginePath(settings, EngineWindow::fldigi + EngineWindow::i1, ui->fldigiEdit->text());
    setEnginePath(settings, EngineWindow::fldigi + EngineWindow::i2, ui->fldigiEdit->text());
    setEnginePath(settings, EngineWindow::gritty + EngineWindow::i1, ui->grittyEdit->text());
    setEnginePath(settings, EngineWindow::gritty + EngineWindow::i2, ui->grittyEdit->text());

    setEngineEnabled(settings, EngineWindow::mmvari + EngineWindow::i1, ui->mmvariEnable1->isChecked());
    setEngineEnabled(settings, EngineWindow::mmvari + EngineWindow::i2, ui->mmvariEnable2->isChecked());
    setEngineEnabled(settings, EngineWindow::mmtty + EngineWindow::i1, ui->mmttyEnable1->isChecked());
    setEngineEnabled(settings, EngineWindow::mmtty + EngineWindow::i2, ui->mmttyEnable2->isChecked());
    setEngineEnabled(settings, EngineWindow::twotone + EngineWindow::i1, ui->twotoneEnable1->isChecked());
    setEngineEnabled(settings, EngineWindow::twotone + EngineWindow::i2, ui->twotoneEnable2->isChecked());
    setEngineEnabled(settings, EngineWindow::fldigi + EngineWindow::i1, ui->fldigiEnable1->isChecked());
    setEngineEnabled(settings, EngineWindow::fldigi + EngineWindow::i2, ui->fldigiEnable2->isChecked());
    setEngineEnabled(settings, EngineWindow::gritty + EngineWindow::i1, ui->grittyEnable1->isChecked());
    setEngineEnabled(settings, EngineWindow::gritty + EngineWindow::i2, ui->grittyEnable2->isChecked());
    setEngineEnabled(settings, EngineWindow::test + EngineWindow::i1, ui->testEnable1->isChecked());
    setEngineEnabled(settings, EngineWindow::test + EngineWindow::i2, ui->testEnable2->isChecked());

    setEnginePort(settings, EngineWindow::gritty + EngineWindow::i1, ui->grittyPort1->text().toInt());
    setEnginePort(settings, EngineWindow::gritty + EngineWindow::i2, ui->grittyPort2->text().toInt());
    setEnginePort(settings, EngineWindow::fldigi + EngineWindow::i1, ui->fldigiPort1->text().toInt());
    setEnginePort(settings, EngineWindow::fldigi + EngineWindow::i2, ui->fldigiPort2->text().toInt());

    setSpeed(settings, "RTTY", ui->rttySpeed->currentText());
    setSpeed(settings, "BPSK", ui->BPSKSpeed->currentText());

    QSettings qsettings;
    qsettings.setValue(geoStr, saveGeometry());

    if (check())
    {
        accept();
    }
}


void EngineConfigure::on_cancelButton_clicked()
{
    reject();
}


void EngineConfigure::on_senderCombo_activated(int /*index*/)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);

    settings.setValue("Sender", ui->senderCombo->currentText());

}

