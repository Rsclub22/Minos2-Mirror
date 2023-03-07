#ifndef GRITTYFRAME_H
#define GRITTYFRAME_H

#include <QFrame>
#include <QProcess>
#include <QTcpSocket>
#include "frequency.h"

namespace Ui {
class GrittyFrame;
}
class QLineEdit;
class QTextEdit;
class EngineWindow;

class GrittyFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GrittyFrame(EngineWindow *parent, QLineEdit *sendEdit, QString fname, QString name);
    ~GrittyFrame();

    void sendCharacters(const QString &, int);
    void sendMode(QString);

    void closeFrame();


private:
    Ui::GrittyFrame *ui;
    EngineWindow *engineWindow;
    QString engineName;
    int grittyPort = 7502;

    QLineEdit *sendEdit = nullptr;
    QProcess *grittyProcess = nullptr;
    bool grittyActive = false;
    QTcpSocket* grittyClient;
    QString msgbuf;
    int markFrequency = 0;
    QString bpskSpeed;
    QString rttySpeed;

    QString fname;
    void createProcess();

    QString getString(QJsonObject o, QString key, QString def);
    bool getBool(QJsonObject o, QString key, bool def);
    int getInt(QJsonObject o, QString key, int def);

    void analyseGrittyMessage(QString m);
private slots:
    void onSendCharacters(QString, int markf);
    void onSetSpeeds(QString b, QString m);

    void onRigModeFreq(QString, Frequency);
    void on_finished(int err, QProcess::ExitStatus exitStatus);
    void on_error(QProcess::ProcessError error);
    void on_readyReadStandardError();
    void on_readyReadStandardOutput();
    void on_started();

    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError error);
    void onReadyRead();

};

#endif // GRITTYFRAME_H
