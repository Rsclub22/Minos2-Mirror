#include <QSettings>

#include "configureknobs.h"
#include "regsettings.h"
#include "ui_configureknobs.h"

ConfigureKnobs::ConfigureKnobs(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConfigureKnobs)
{
    ui->setupUi(this);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("knobs/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->knobCombo->addItem(tr("None"));

#if defined (WIN32)
    ui->knobCombo->addItem(tr("FlexControl"));
    ui->knobCombo->addItem(tr("Griffin PowerMate"));
#endif

    curKnob = settings.getSettings().value("knobs/curKnob", 0).toInt();
    ui->knobCombo->setCurrentIndex(curKnob);
    on_knobCombo_activated(curKnob);
}

ConfigureKnobs::~ConfigureKnobs()
{
    delete ui;
}

void ConfigureKnobs::start()
{
    ControlFlex::getControlFlex()->start();
    ControlPowerMate::getControlPowerMate()->start();
}
void ConfigureKnobs::stop()
{
    ControlFlex::getControlFlex()->stop();
    ControlPowerMate::getControlPowerMate()->stop();
}
void ConfigureKnobs::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("knobs/geometry", saveGeometry());
}

void ConfigureKnobs::done(int r)
{
    if(QDialog::Accepted == r)  // ok was pressed
    {

    }
    doCloseEvent();
    QDialog::done(r);
}

void ConfigureKnobs::closeEvent (QCloseEvent *event)
{
    // called when closed by "X"
    doCloseEvent();
    QWidget::closeEvent(event);
}

void ConfigureKnobs::on_pushButton_clicked()
{
    accept();
}


void ConfigureKnobs::on_knobCombo_activated(int index)
{
    curKnob = index;
    if (index == 0)
    {
        // none
        ui->flexControlFrame->setVisible(false);
        ui->powerMateFrame->setVisible(false);
        ui->noneFrame->setVisible(true);
    }
    else if (index == 1)
    {
        // FlexControl
        ui->noneFrame->setVisible(false);
        ui->powerMateFrame->setVisible(false);
        ui->flexControlFrame->setVisible(true);
    }
    else if (index == 2)
    {
        // PowerMate
        ui->noneFrame->setVisible(false);
        ui->flexControlFrame->setVisible(false);
        ui->powerMateFrame->setVisible(true);
    }

    RegSettings settings;
    settings.getSettings().setValue("knobs/curKnob", curKnob);
}

