#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "StdInReader.h"

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
    StdInReader *stdinReader = new StdInReader(this);

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

private slots:
    void CloseTimerTimer();
};

#endif // MAINWINDOW_H
