#include <QSettings>

//#include "monitoredlogs.h"
#include "kstmonitoredlogs.h"
#include "ui_kstmonitoredlogs.h"

KSTMonitoredLogs::KSTMonitoredLogs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KSTMonitoredLogs)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint));

    QSettings settings;
    QByteArray geometry = settings.value("geometry/Monitor").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    hide();

    connect(ui->treeView, &MonitoredLogs::logStarted, this, &KSTMonitoredLogs::logStarted);
//            [=](QSharedPointer<MonitoredLog> ml){emit logStarted(ml);});

    connect(ui->treeView, &MonitoredLogs::logClosed, this, &KSTMonitoredLogs::logClosed);
//            [=](QSharedPointer<MonitoredLog> ml){emit logClosed(ml);});
}

KSTMonitoredLogs::~KSTMonitoredLogs()
{
    delete ui;
}
void KSTMonitoredLogs::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Monitor", saveGeometry());
    QWidget::resizeEvent(event);
}
void KSTMonitoredLogs::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Monitor", saveGeometry());
    QWidget::moveEvent(event);
}
void KSTMonitoredLogs::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry/Monitor", saveGeometry());
    }
}
void KSTMonitoredLogs::on_hideButton_clicked()
{
    hide();
}

