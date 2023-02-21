#ifndef GRITTYFRAME_H
#define GRITTYFRAME_H

#include "frequency.h"
#include <QFrame>
#include <QProcess>
#include <QTcpSocket>

namespace Ui {
class GrittyFrame;
}
class QLineEdit;
class QTextEdit;

class GrittyFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GrittyFrame(QWidget *parent, QLineEdit *sendEdit, QString fname);
    ~GrittyFrame();

    void sendCharacters(const QString &, int);
    void sendMode(QString);

    void closeFrame();


private:
    Ui::GrittyFrame *ui;
    QLineEdit *sendEdit = nullptr;
    QProcess *grittyProcess = nullptr;
    bool grittyActive = false;
    QTcpSocket* grittyClient;
    QString msgbuf;
    int carrier = 0;
    int bpskSpeed = 31;
    int rttySpeed = 45;

    QString fname;
    void createProcess();

    QString getString(QJsonObject o, QString key, QString def);
    bool getBool(QJsonObject o, QString key, bool def);
    int getInt(QJsonObject o, QString key, int def);

    void analyseGrittyMessage(QString m);
private slots:
    void onSendCharacters(QString, int carrier);
    void onSetSpeeds(int b, int m);

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
