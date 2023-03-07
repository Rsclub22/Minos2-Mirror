#ifndef FLDIGIFRAME_H
#define FLDIGIFRAME_H

#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>

#include <QProcess>

#include "frequency.h"
#include "maiaXmlRpcClient.h"
class EngineWindow;

namespace Ui {
class FLDigiFrame;
}
class QTimer;
class FLDigiFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FLDigiFrame(EngineWindow *parent,  QLineEdit *sendEdit, QString fname, QString name);
    ~FLDigiFrame();

    void sendCharacters(const QString &, int mfreq);
    void sendMode(QString);
    void closeFrame();

private:
    Ui::FLDigiFrame *ui;
    EngineWindow *engineWindow;
    QString engineName;
    MaiaXmlRpcClient *rpcClient = nullptr;
    QLineEdit *sendEdit = nullptr;
    QProcess *fldigiProcess = nullptr;
    bool fldigiActive = false;
    QString fname;
    QTimer *getTimer;
    int markFrequency = 0;
    QString mode;
    int carrierOffsetFromMark = 0;
    QString bpskSpeed;
    QString rttySpeed;

    void createProcess();

    void addText(const QString &t);
private slots:
    void onSendCharacters(QString, int);
    void onSetSpeeds(QString b, QString m);

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
