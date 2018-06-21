#include "MinosRPC.h"

#include "ChatFrame.h"
#include "ui_ChatFrame.h"

ChatFrame::ChatFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ChatFrame)
{
    ui->setupUi(this);

    connect (ChatServer::getChatServer(), SIGNAL(ChatServerList(QVector<Server>)), this, SLOT(ChatServerList(QVector<Server>)));
    connect (ChatServer::getChatServer(), SIGNAL(ChatMessages(QVector<QString>)), this, SLOT(ChatMessages(QVector<QString>)));
}

ChatFrame::~ChatFrame()
{
    delete ui;
}
//---------------------------------------------------------------------------
void ChatFrame::ChatServerList(QVector<Server> serverList)
{
    ui->StationList->clear();
    for ( QVector<Server>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        QString state = stateIndicator[(*i).state] + " " + (*i).name;
        ui->StationList->addItem( state );
    }
}

void ChatFrame::ChatMessages(QVector<QString> chatQueue)
{
    for ( QVector<QString>::iterator i = chatQueue.begin(); i != chatQueue.end(); i++ )
    {
       ui->ChatMemo->append( (*i) );
       trace("syncChat " + (*i));
    }
    chatQueue.clear();
}
//---------------------------------------------------------------------------

void ChatFrame::on_SendButton_clicked()
{
    ChatServer::getChatServer()->sendMessage(ui->ChatEdit->text());

    ui->ChatEdit->clear(); // otherwise it is a pain!
    ui->ChatEdit->setFocus();
}
void ChatFrame::keyPressEvent( QKeyEvent* event )
{
    int Key = event->key();

    if (Key == Qt::Key_Return || Key == Qt::Key_Enter)
    {
        ui->SendButton->clicked();
    }
    if (Key == Qt::Key_Escape)
    {
        ui->ChatEdit->clear();
    }
}
