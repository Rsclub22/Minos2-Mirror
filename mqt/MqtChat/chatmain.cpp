#include "base_pch.h"
#include "MinosRPC.h"

#include "chatmain.h"
#include "ui_chatmain.h"

TMinosChatForm::TMinosChatForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // force the app BEFORE constructing the ui - which will
    // initialise it through the chat server
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName(), false);
    Q_UNUSED(rpc);

    ui->setupUi(this);

    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createCloseEvent();
    connect(&CloseTimer, SIGNAL(timeout()), this, SLOT(CloseTimerTimer()));
    CloseTimer.start(100);

    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->chatFrame->setStandAlone();

}

TMinosChatForm::~TMinosChatForm()
{
    delete ui;
}
void TMinosChatForm::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(false);
}
void TMinosChatForm::CloseTimerTimer(  )
{
   static bool closed = false;
   if ( !closed )
   {
      if ( checkCloseEvent() )
      {
         closed = true;
         close();
      }
   }
}

//---------------------------------------------------------------------------

void TMinosChatForm::closeEvent(QCloseEvent *event)
{
    // and tidy up all loose ends

    QWidget::closeEvent(event);
}
void TMinosChatForm::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void TMinosChatForm::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void TMinosChatForm::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
