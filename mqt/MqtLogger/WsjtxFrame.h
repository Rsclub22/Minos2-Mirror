#ifndef WSJTXFRAME_H
#define WSJTXFRAME_H

#include "base_pch.h"
#include <QFrame>

namespace Ui {
class WsjtxFrame;
}
class WsjtServer
{
public:
    QString serverName;
    QString app;
    PublishState state;
};
class WsjtxFrame : public QFrame
{
    Q_OBJECT

public:
    explicit WsjtxFrame(QWidget *parent = nullptr);
    ~WsjtxFrame();

    void setContest(BaseContestLog *c);


private:
    Ui::WsjtxFrame *ui;
    BaseContestLog *ct = nullptr;
    QVector<WsjtServer> serverList;


private slots:
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
    void on_notify( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );


};

#endif // WSJTXFRAME_H
