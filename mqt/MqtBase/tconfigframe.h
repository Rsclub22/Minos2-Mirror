#ifndef TCONFIGFRAME_H
#define TCONFIGFRAME_H

#include "base_pch.h"

namespace Ui {
class TConfigFrame;
}

typedef void ( *ConfigCloseCallBack ) ( QWidget *w );
class ConfigElementFrame;

class TConfigFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TConfigFrame(QWidget *parent = nullptr);
    ~TConfigFrame();
    void initialise(QWidget *parent, ConfigCloseCallBack, bool showAutoStart);
    void setup(bool started);

    void start();

private slots:
    void on_StartButton_clicked();

    void on_StopButton_clicked();

    void on_SetButton_clicked();

    void on_autoStartCheckBox_clicked();

    void on_SaveCloseButton_clicked();

    void on_newElementButton_clicked();

    void afterFrameShown();

public slots:
    void on_CancelButton_clicked();

signals:
    void frameShown();

protected:
    virtual void showEvent(QShowEvent *ev);

private:
    Ui::TConfigFrame *ui;
    QWidget *parent;
    QVector<ConfigElementFrame *> elementFrames;

    ConfigCloseCallBack closeCb;

    void saveAll();
};
#endif // TTCONFIGFRAME_H
