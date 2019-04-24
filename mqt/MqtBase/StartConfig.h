#ifndef STARTCONFIG_H
#define STARTCONFIG_H

#include "base_pch.h"
#include <QTimer>

namespace Ui {
class StartConfig;
}
class ConfigElementFrame;

class StartConfig : public QDialog
{
    Q_OBJECT

public:
    explicit StartConfig(QWidget *parent, bool showAutoStart);
    virtual ~StartConfig() override;

private:
    Ui::StartConfig *ui;
    QTimer runTimer;
    QTimer formShowTimer;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

public Q_SLOTS:
   virtual void accept() override;
   virtual void reject() override;


public:
    void setup(bool started);

    void start();
private slots:
    void on_StartButton_clicked();

    void on_StopButton_clicked();

    void on_SetButton_clicked();

    void on_autoStartCheckBox_clicked();

    void on_SaveCloseButton_clicked();

    void on_newElementButton_clicked();

    void checkEnabled();
    void on_formShown();
public slots:
    void on_CancelButton_clicked();

private:
    QVector<ConfigElementFrame *> elementFrames;

    void copyFromScreen();

    void saveAll();

};

#endif // STARTCONFIG_H
