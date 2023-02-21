#ifndef FLDIGIFRAME_H
#define FLDIGIFRAME_H

#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>

#include <QProcess>

#include "frequency.h"
#include "maiaXmlRpcClient.h"

namespace Ui {
class FLDigiFrame;
}
class QTimer;
class FLDigiFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FLDigiFrame(QWidget *parent,  QLineEdit *sendEdit, QString fname);
    ~FLDigiFrame();

    void sendCharacters(const QString &, int carrier);
    void sendMode(QString);
    void closeFrame();

private:
    Ui::FLDigiFrame *ui;
    MaiaXmlRpcClient *rpcClient = nullptr;
    QLineEdit *sendEdit = nullptr;
    QProcess *fldigiProcess = nullptr;
    bool fldigiActive = false;
    QString fname;
    QTimer *getTimer;
    int carrier = 0;
    QString mode;
    int carrierOffset = 0;
    int bpskSpeed = 31;
    int rttySpeed = 45;

    void createProcess();

    void addText(const QString &t);
private slots:
    void onSendCharacters(QString, int);
    void onSetSpeeds(int b, int m);

    void onRigModeFreq(QString, Frequency);
    void on_finished(int err, QProcess::ExitStatus exitStatus);
    void on_error(QProcess::ProcessError error);
    void on_readyReadStandardError();
    void on_readyReadStandardOutput();
    void on_started();

    void myResponseMethod(QVariant&);
    void myCarrierResponseMethod(QVariant&);
    void myTxResponseMethod(QVariant&);
    void myRxResponseMethod(QVariant&);
    void myFaultResponse(int, const QString &);

    void onGetTimer();
signals:
    void txChanged(bool);
};

#endif // FLDIGIFRAME_H
