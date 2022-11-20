#ifndef DMMAINWINDOW_H
#define DMMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "StdInReader.h"

#include "MMVARIFrame.h"
#include "MMTTYFrame.h"
#include "FLDigiFrame.h"


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

    MMVARIFrame *mmvariFrame = nullptr;
    MMTTYFrame *mmttyFrame = nullptr;
    FLDigiFrame *fldigiFrame = nullptr;

    void closeAllEngines();


    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void doCloseEvent();
protected:
    virtual void showEvent(QShowEvent *) override;

private slots:
    void LogTimerTimer();

    void onStdInRead(QString cmd);

    void on_actionMMVARI_triggered();

    void on_actionMMTTY_triggered();

    void on_action2Tone_triggered();

    void on_actionFLDigi_triggered();

    void on_actionExit_triggered();

    void on_sendButton_clicked();


};
#endif // DMMAINWINDOW_H
