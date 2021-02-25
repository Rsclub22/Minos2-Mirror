#ifndef CHATFRAME_H
#define CHATFRAME_H

#include "base_pch.h"

#include "ChatServer.h"

namespace Ui {
class ChatFrame;
}

class ChatFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ChatFrame(QWidget *parent = nullptr);
    ~ChatFrame() override;

private:
    Ui::ChatFrame *ui;
    void keyPressEvent( QKeyEvent* event ) override;

private slots:
    void on_SendButton_clicked();

    void ChatServerList(QVector<ChatServerApp>);
    void ChatMessages(QVector<QString>);
    void on_FontChanged();
};

#endif // CHATFRAME_H
