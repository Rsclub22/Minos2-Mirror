#ifndef CHANGENAME_H
#define CHANGENAME_H

#include "base_pch.h"
#include "qttelnet.h"

namespace Ui {
class ChangeName;
}

class ChangeName : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeName(QWidget *parent = nullptr);
    ~ChangeName();

    void connectToHost();

    QString newName;
    QtTelnet* tnclient = 0;

    QString serverName;
    QString serverPort;
    QString myCallsign;
    QString password;

    int kstChatSelection = 0;

private:
    Ui::ChangeName *ui;

    bool userLoggedIn = false;
    bool setupComplete = false;
    bool nameChanged = false;

    void doChangeName();
    void sendData(QString);

private slots:
    void connectionEstablished();
    void connectionError(QAbstractSocket::SocketError error);
    void loggedOut();
    void messageRx(QString msg);

};

#endif // CHANGENAME_H
