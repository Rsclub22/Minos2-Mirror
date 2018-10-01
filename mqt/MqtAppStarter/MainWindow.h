#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "base_pch.h"
//#include <QMainWindow>
#include "StartConfig.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void startTimer_Timeout();

    void on_appsButton_clicked();

    void on_closeButton_clicked();

    void on_stdOutLine(QString);

private:
    Ui::MainWindow *ui;
    QTimer startTimer;
    QMetaObject::Connection m_connection;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void start();
};

#endif // MAINWINDOW_H
