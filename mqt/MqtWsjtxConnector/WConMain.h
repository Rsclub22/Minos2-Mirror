#ifndef WCONMAIN_H
#define WCONMAIN_H

#include "base_pch.h"
#include <QMainWindow>

namespace Ui {
class WConMain;
}

class WsjtServer
{
public:
    QString serverName;
    QString app;
    PublishState state;
};

class WConMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit WConMain(QWidget *parent = nullptr);
    virtual ~WConMain() override;

private slots:
    void onStdInRead(QString);
    void on_pushButton_clicked();
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
    void on_notify( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );
    void LogTimerTimer( );

private:
    Ui::WConMain *ui;

    StdInReader stdinReader;
    QTimer LogTimer;
    QVector<WsjtServer> serverList;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
};

#endif // WCONMAIN_H
