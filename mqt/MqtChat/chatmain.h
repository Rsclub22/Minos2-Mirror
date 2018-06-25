#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "base_pch.h"
#include "ChatServer.h"

namespace Ui {
class MainWindow;
}

class TMinosChatForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit TMinosChatForm(QWidget *parent = nullptr);
    ~TMinosChatForm() override;

private:
    Ui::MainWindow *ui;
    QTimer CloseTimer;
    StdInReader stdinReader;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

private slots:
    void onStdInRead(QString);
    void CloseTimerTimer();
};

#endif // MAINWINDOW_H
