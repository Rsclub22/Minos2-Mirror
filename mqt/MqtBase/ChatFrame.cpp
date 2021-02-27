#include "MinosRPC.h"
#include "MinosLoggerEvents.h"

#include "ChatFrame.h"
#include "ui_ChatFrame.h"

ChatFrame::ChatFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ChatFrame)
{
    ui->setupUi(this);

    connect (ChatServer::getChatServer(), SIGNAL(ChatServerList(QVector<ChatServerApp>)), this, SLOT(ChatServerList(QVector<ChatServerApp>)));
    connect (ChatServer::getChatServer(), SIGNAL(ChatMessages(QVector<QString>)), this, SLOT(ChatMessages(QVector<QString>)));
    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    on_FontChanged();
}

ChatFrame::~ChatFrame()
{
    delete ui;
}

void ChatFrame::on_FontChanged()
{
    QFont cf = QApplication::font();
    ui->StationList->setFont(cf);
}

//---------------------------------------------------------------------------
void ChatFrame::ChatServerList(QVector<ChatServerApp> serverList)
{
    ui->StationList->clear();
    for ( auto const &i: serverList )
    {
        Frequency f = i.freq;
        QString state = ChatServer::tr(ChatServer::stateIndicator[i.state]) + " " + i.app + "\r\n" + f.str();
        ui->StationList->addItem( state );
    }
}

void ChatFrame::ChatMessages(QVector<QString> chatQueue)
{
    for ( auto const &i: chatQueue )
    {
       ui->ChatMemo->append( i );
       trace("syncChat " + i);
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
        QMetaObject::invokeMethod(ui->SendButton, "clicked");
    }
    else
    {
        // default handler for event
        QFrame::keyPressEvent(event);
    }
}
