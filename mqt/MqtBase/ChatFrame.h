#ifndef CHATFRAME_H
#define CHATFRAME_H

#include "base_pch.h"

namespace Ui {
class ChatFrame;
}
class Server
{
   public:
      QString name;
      QString app;
      QString ip;
      PublishState state;
};
class ChatFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ChatFrame(QWidget *parent = nullptr);
    ~ChatFrame() override;

private:
    Ui::ChatFrame *ui;
    QVector<Server> serverList;
    QTimer SyncTimer;

    void addChat(const QString &mess);
    void syncChat();
    void syncStations();
    void keyPressEvent( QKeyEvent* event ) override;

private slots:
    void SyncTimerTimer( );

    void on_SendButton_clicked();

    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
    void on_notify( bool err, QSharedPointer<MinosRPCObj>, const QString &from );

};

#endif // CHATFRAME_H
