#ifndef DMMAINWINDOW_H
#define DMMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "StdInReader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DMMainWindow; }
QT_END_NAMESPACE

class DMMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    DMMainWindow(QWidget *parent = nullptr);
    virtual ~DMMainWindow() override;

private:
    Ui::DMMainWindow *ui;
    StdInReader *stdinReader = new StdInReader(this);


    QTimer LogTimer;
    QString geoStr;         // geometry registry location

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;


private slots:
    void LogTimerTimer();

    void onStdInRead(QString cmd);

};
#endif // DMMAINWINDOW_H
