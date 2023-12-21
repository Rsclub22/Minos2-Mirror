#include <QSettings>
#include <QFileDialog>

#include "regsettings.h"
#include "dmmainwindow.h"
#include "enginewindow.h"
#include "fileutils.h"
#include "rigcommon.h"

#include "engineconfigure.h"
#include "ui_engineconfigure.h"

void EngineConfigure::setEnginePath(QSettings &settings, QString engine, QString path)
{
    QString eStr = QString("engines/");
    settings.setValue(eStr + engine, path);
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
QString EngineConfigure::getEnginePTT(QSettings &settings, QString engine)
{
    QString s = settings.value("PTT/" + engine).toString();
    return s;
}
QString EngineConfigure::getEnginePTT(QString engine)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    return getEnginePTT(settings, engine);
}
void EngineConfigure::setEnginePTT(QSettings &settings,QString engine, QString port)
{
    settings.setValue("PTT/" + engine, port);
}

int EngineConfigure::getEnginePTTL(QSettings &settings, QString engine)
{
    int p = settings.value("PTTL/" + engine).toInt();
    return p;
}
int EngineConfigure::getEnginePTTL(QString engine)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);
    return getEnginePTTL(settings, engine);
}
void EngineConfigure::setEnginePTT(QSettings &settings,QString engine, int l)
{
    settings.setValue("PTTL/" + engine, l);
}

void EngineConfigure::checkEnginePath(QLineEdit *ele, QCheckBox *ecb)
{
    QString m = ele->text();
    QString ext = ExtractFileExt(m);
    bool ok = false;
    if (ext.compare("ocx", Qt::CaseInsensitive))
    {
        ok = FileExists(m);
    }
    else
    {
        ok = FileExecutable(m);
    }
    if (!ok)
    {
        ecb->setChecked(false);
        ecb->setEnabled(false);
    }
    else
    {
        ecb->setEnabled(true);
    }
}
void EngineConfigure::checkEnginePort(QLineEdit *ele, QCheckBox *ecb)
{
    int p = ele->text().toInt();
    if (p <= 0)
    {
        ecb->setChecked(false);
        ecb->setEnabled(false);
    }
    else
    {
        ecb->setEnabled(true);
    }
}
void EngineConfigure::setPortDefault(QSettings &settings, QString engine, QLineEdit *ple, int def)
{
    int p = getEnginePort(settings, engine);
    if (p == 0)
    {
        p = def;
    }
    ple->setText(QString::number(p));
    ple->setValidator(new QIntValidator(0, 0xffff, this));

}
bool EngineConfigure::check()
{
    // check

    // all paths exist

    // nothing enabled for which there is no path

    // ports are set where required

    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);

    checkEnginePath(ui->mmvariEdit, ui->mmvariEnable1);
    checkEnginePath(ui->mmvariEdit, ui->mmvariEnable2);

    checkEnginePath(ui->twotoneEdit1, ui->twotoneEnable1);
    checkEnginePath(ui->twotoneEdit2, ui->twotoneEnable2);

// and check not the same

    checkEnginePath(ui->mmttyEdit1, ui->mmttyEnable1);
    checkEnginePath(ui->mmttyEdit2, ui->mmttyEnable2);

    checkEnginePath(ui->fldigiEdit, ui->fldigiEnable1);
    checkEnginePath(ui->fldigiEdit, ui->fldigiEnable2);

    checkEnginePath(ui->grittyEdit, ui->grittyEnable1);
    checkEnginePath(ui->grittyEdit, ui->grittyEnable2);

    checkEnginePort(ui->fldigiPort1, ui->fldigiEnable1);
    checkEnginePort(ui->fldigiPort2, ui->fldigiEnable2);

    checkEnginePort(ui->grittyPort1, ui->grittyEnable1);
    checkEnginePort(ui->grittyPort2, ui->grittyEnable2);

    return true;
}
EngineConfigure::EngineConfigure(DMMainWindow *parent) :
    QDialog(parent),
    ui(new Ui::EngineConfigure),
    mainWindow(parent)
{
    ui->setupUi(this);

#ifndef Q_OS_WIN
    ui->MMTTYframe->setVisible(false);
    ui->grittyframe->setVisible(false);
    ui->MMVARI1frame->setVisible(false);
    ui->MMVARI2frame->setVisible(false);
    ui->twotoneframe->setVisible(false);
#endif


    connect(ui->mmttyEdit1, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->mmttyEdit2, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->twotoneEdit1, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->twotoneEdit2, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->grittyEdit, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->fldigiEdit, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->grittyPort1, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->grittyPort2, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->grittyPort1, &QLineEdit::editingFinished, this, &EngineConfigure::check);
    connect(ui->grittyPort2, &QLineEdit::editingFinished, this, &EngineConfigure::check);

    RegSettings qsettings;
    geoStr = QString("dataModes/Configuration/geometry");
    QByteArray geometry = qsettings.getSettings().value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


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

    fillPortsInfo(ui->MMVARIPTT1);
    QString p = getEnginePTT(settings, EngineWindow::mmvari + EngineWindow::i1);
    int l = getEnginePTTL(settings, EngineWindow::mmvari + EngineWindow::i1);

    ui->MMVARIPTT1->setCurrentText(p);
    if (l == 1)
    {
        ui->MMVARIDTR1->setChecked(true);
    }
    else
    {
        ui->MMVARIRTS1->setChecked(true);
    }
    fillPortsInfo(ui->MMVARIPTT2);
    p = getEnginePTT(settings, EngineWindow::mmvari + EngineWindow::i2);
    l = getEnginePTTL(settings, EngineWindow::mmvari + EngineWindow::i2);

    ui->MMVARIPTT2->setCurrentText(p);
    if (l == 1)
    {
        ui->MMVARIDTR2->setChecked(true);
    }
    else
    {
        ui->MMVARIRTS2->setChecked(true);
    }

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

    setPortDefault(settings, EngineWindow::fldigi + EngineWindow::i1, ui->fldigiPort1, 7362);
    setPortDefault(settings, EngineWindow::fldigi + EngineWindow::i2, ui->fldigiPort2, 7363);
    setPortDefault(settings, EngineWindow::gritty + EngineWindow::i1, ui->grittyPort1, 7502);
    setPortDefault(settings, EngineWindow::gritty + EngineWindow::i2, ui->grittyPort2, 7503);

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

    check();
}

EngineConfigure::~EngineConfigure()
{
    delete ui;
}
void EngineConfigure::closeEvent(QCloseEvent *event)
{
    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());
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
    check();
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

    if (check())
    {
        QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);

        setEngineSound(settings, EngineWindow::mmvari + EngineWindow::i1, "input", ui->MMVARIRX1->currentText());
        setEngineSound(settings, EngineWindow::mmvari + EngineWindow::i1, "output", ui->MMVARITX1->currentText());
        setEnginePTT(settings, EngineWindow::mmvari + EngineWindow::i1, ui->MMVARIPTT1->currentText());
        setEnginePTT(settings, EngineWindow::mmvari + EngineWindow::i2, ui->MMVARIPTT2->currentText());
        setEnginePTT(settings, EngineWindow::mmvari + EngineWindow::i1, ui->MMVARIDTR1->isChecked());
        setEnginePTT(settings, EngineWindow::mmvari + EngineWindow::i2, ui->MMVARIDTR2->isChecked());

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

        accept();
    }
    RegSettings qsettings;
    qsettings.getSettings().setValue(geoStr, saveGeometry());
}


void EngineConfigure::on_cancelButton_clicked()
{
    reject();
}


void EngineConfigure::on_senderCombo_activated(int /*index*/)
{
    QSettings settings("./Configuration/DataModes.ini", QSettings::IniFormat);

    settings.setValue("Sender", ui->senderCombo->currentText());

    check();

}

