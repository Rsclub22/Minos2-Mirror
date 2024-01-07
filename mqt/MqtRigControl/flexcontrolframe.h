#ifndef FLEXCONTROLFRAME_H
#define FLEXCONTROLFRAME_H

#include <QFrame>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class QComboBox;

namespace Ui {
class FlexControlFrame;
}

class FlexControlFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FlexControlFrame(QWidget *parent = nullptr);
    ~FlexControlFrame();

private slots:

    void on_enabledCb_clicked();
    void on_comPort_activated(const QString &arg1);

private:
    Ui::FlexControlFrame *ui;
    void fillPortsInfo(QComboBox *cb);
};

class ControlFlex:public QObject
{
    Q_OBJECT
public:

    static ControlFlex *getControlFlex()
    {
        if (!cf)
        {
            cf = new ControlFlex();
        }
        return cf;
    }

    void start();
    void stop();
private slots:
    void on_readyRead();

    void errorOccurred(QSerialPort::SerialPortError error);
signals:
    void dataReceived(QByteArray);
    void errString(QString);

private:
    static ControlFlex *cf;

    QSerialPort *sp = nullptr;

    bool openFlag = false;

    ControlFlex();
    ~ControlFlex() override;

};

#endif // FLEXCONTROLFRAME_H
