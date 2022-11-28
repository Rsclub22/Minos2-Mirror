#ifndef FLDIGIFRAME_H
#define FLDIGIFRAME_H

#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>

#include <QProcess>

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

    void sendCharacters(const QString &);
    void closeFrame();

private:
    Ui::FLDigiFrame *ui;
    MaiaXmlRpcClient *rpcClient = nullptr;
    QLineEdit *sendEdit = nullptr;
    QProcess *fldigiProcess = nullptr;
    bool fldigiActive = false;
    QString fname;
    QTimer *getTimer;

    void createProcess();

private slots:
    void on_finished(int err, QProcess::ExitStatus exitStatus);
    void on_error(QProcess::ProcessError error);
    void on_readyReadStandardError();
    void on_readyReadStandardOutput();
    void on_started();

    void myResponseMethod(QVariant&);
    void myFaultResponse(int, const QString &);

    void onGetTimer();
};

#endif // FLDIGIFRAME_H
