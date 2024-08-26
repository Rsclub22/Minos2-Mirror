#ifndef DMMAINWINDOW_H
#define DMMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMap>

#include "CommandReader.h"
#include "XMPPRPCObj.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DMMainWindow; }
QT_END_NAMESPACE

class QPushButton;
class MonitoredLog;
class MonitoredLogs;
class EngineWindow;

class DMMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    DMMainWindow(QWidget *parent = nullptr);
    virtual ~DMMainWindow() override;

    unsigned int inChannels = 0;
    unsigned int outChannels = 0;

    QStringList inputDevices;
    QStringList outputDevices;

    QMap<QString, int> inDeviceIds;
    QMap<QString, int> outDeviceIds;

    QString configFile;

private:
    Ui::DMMainWindow *ui;
    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));

    MonitoredLogs *logsTreeView = nullptr;

    QTimer *watchDog = nullptr;

    QTimer LogTimer;
    QString geoStr;         // geometry registry location

    QVector<EngineWindow *> engines;
    void closeAllEngines();

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void doCloseEvent();

    void testAutoStart();
    void onNewStanzas();

private slots:
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);

    void LogTimerTimer();

    void onCommandRead(QString cmd);

    void onNewLog(QSharedPointer<MonitoredLog> ml);

    void on_dataConfigBrowse_clicked();

    void on_configureButton_clicked();
    void on_startButton_clicked();
    void sendPressed(QString d, int c);

    void onLogChanged(QSharedPointer<MonitoredLog> ml);
    void onLogStarted(QSharedPointer<MonitoredLog> ml);
    void onLogClosed(QSharedPointer<MonitoredLog> ml);
    void engineDestroyed(QObject *d);

signals:
    void setSpeeds(QString b, QString r);

};
extern DMMainWindow *mainWindow;

#endif // DMMAINWINDOW_H
