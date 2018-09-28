#ifndef CONTROLMAIN_H
#define CONTROLMAIN_H

#include "base_pch.h"
#include <QMainWindow>
#include "CommonMonitor.h"
#include "controlport.h"
#include "ConfigFile.h"
#include "RigCache.h"

namespace Ui {
class ControlMain;
}

class ControlMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControlMain(QWidget *parent = nullptr);
    ~ControlMain() override;
    commonController monitor;

    void setPTTIn(bool s);
    void setPTTOut(bool s);
    void setL1(bool s);
    void setL2(bool s);
    void setL3(bool s);
    void setL4(bool s);
    void setL5(bool s);
    void setL6(bool s);
    void setT1(bool s);
    void setT2(bool s);

private:
    Ui::ControlMain *ui;
    void logMessage( QString s );
    void doProcLineEvents( );
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void subscribeApps();

    RigCache rigCache;
    QMap<QString,QVector< QSharedPointer<Connectable> > > catMap;
    QVector<QSharedPointer<Connectable> > connectables;
    QVector<QString> servers;
    PubSubName rigSelected;



    StdInReader stdinReader;
    QTimer LogTimer;
    QTimer formShowTimer;

private slots:
    void onStdInRead(QString);

    void on_formShown();
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
    void on_notify( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );
    void LogTimerTimer( );
    void linesChangedEvent();
};

extern ControlMain *controlMain;
#endif // CONTROLMAIN_H
