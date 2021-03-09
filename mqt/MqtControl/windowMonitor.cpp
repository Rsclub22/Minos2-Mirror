#include "windowMonitor.h"
#include "ui_windowMonitor.h"
#include "ControlMain.h"

windowMonitor::windowMonitor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::windowMonitor)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    QSettings settings;
    QByteArray geometry = settings.value("monitorGeometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);
    else
    {
        setPosTimer.setSingleShot(true);
        connect(&setPosTimer, &QTimer::timeout, this, &windowMonitor::setPosition);
        setPosTimer.start(100);
    }
}
void windowMonitor::setPosition()
{
    QPoint tl = controlMain->frameGeometry().topLeft();
    tl.setY(tl.y() + controlMain->frameGeometry().height() + 30);
    move(tl);
}

windowMonitor::~windowMonitor()
{
    delete ui;
}
void windowMonitor::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("monitorGeometry", saveGeometry());
    QWidget::moveEvent(event);
}
void windowMonitor::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("monitorGeometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void windowMonitor::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("monitorGeometry", saveGeometry());
    }
}
bool windowMonitor::L1Checked()
{
    return ui->L1CheckBox->isChecked();
}

bool windowMonitor::L2Checked()
{
    return ui->L2CheckBox->isChecked();
}

bool windowMonitor::L3Checked()
{
    return ui->L3CheckBox->isChecked();
}

bool windowMonitor::L4Checked()
{
    return ui->L4CheckBox->isChecked();
}

bool windowMonitor::L5Checked()
{
    return ui->L5CheckBox->isChecked();
}

bool windowMonitor::L6Checked()
{
    return ui->L6CheckBox->isChecked();
}

bool windowMonitor::PTTChecked()
{
    return ui->PTTInCheckBox->isChecked();
}

void windowMonitor::checkL1(bool checked)
{
    ui->L1CheckBox->setChecked(checked);
}

void windowMonitor::checkL2(bool checked)
{
    ui->L2CheckBox->setChecked(checked);
}

void windowMonitor::checkL3(bool checked)
{
    ui->L3CheckBox->setChecked(checked);
}

void windowMonitor::checkL4(bool checked)
{
    ui->L4CheckBox->setChecked(checked);
}

void windowMonitor::checkL5(bool checked)
{
    ui->L5CheckBox->setChecked(checked);
}

void windowMonitor::checkL6(bool checked)
{
    ui->L6CheckBox->setChecked(checked);
}

void windowMonitor::checkPTT(bool checked)
{
    ui->PTTInCheckBox->setChecked(checked);
}

