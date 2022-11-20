#include <QSettings>
#include <QTimer>

#include "AppStartup.h"
#include "MinosRPC.h"
#include "LogEvents.h"
#include "MTrace.h"

#include "dmmainwindow.h"
#include "ui_dmmainwindow.h"

DMMainWindow::DMMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DMMainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(stdinReader, &StdInReader::stdinLine, this, &DMMainWindow::onStdInRead);

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    Q_UNUSED(rpc)

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &DMMainWindow::LogTimerTimer);
    LogTimer.start(100);

    QSettings settings;
    geoStr = QString("clusterServer/geometry");
    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

}

DMMainWindow::~DMMainWindow()
{
    delete ui;
}
void DMMainWindow::LogTimerTimer()
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
void DMMainWindow::closeEvent(QCloseEvent *event)
{
    doCloseEvent();
    QWidget::closeEvent(event);
}
void DMMainWindow::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    QWidget::moveEvent(event);
}
void DMMainWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    QWidget::resizeEvent(event);
}
void DMMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue(geoStr, saveGeometry());
    }
}

void DMMainWindow::onStdInRead(QString cmd)
{
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        close();
    }
}
void DMMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);


}
void DMMainWindow::closeAllEngines()
{
    if (mmvariFrame)
    {
        delete mmvariFrame;
        mmvariFrame = nullptr;
    }

    ui->actionMMVARI->setChecked(false);

    if (mmttyFrame)
    {
        mmttyFrame->closeFrame();

        mmttyFrame->deleteLater();
        mmttyFrame = nullptr;
    }

    ui->actionMMTTY->setChecked(false);
    ui->action2Tone->setChecked(false);

    if (fldigiFrame)
    {
        fldigiFrame->closeFrame();
        fldigiFrame->deleteLater();
        fldigiFrame = nullptr;

    }
}

void DMMainWindow::on_actionMMVARI_triggered()
{
    closeAllEngines();

    ui->actionMMVARI->setChecked(true);

    mmvariFrame = new MMVARIFrame(this, dynamic_cast<QVBoxLayout *>(ui->centralwidget->layout()), ui->rxChars, ui->sendEdit);
}

void DMMainWindow::on_actionMMTTY_triggered()
{
    closeAllEngines();
    mmttyFrame = new MMTTYFrame(false, ui->rxChars, ui->sendEdit);
    ui->actionMMTTY->setChecked(true);
}

void DMMainWindow::on_action2Tone_triggered()
{
    closeAllEngines();
    mmttyFrame = new MMTTYFrame(true, ui->rxChars, ui->sendEdit);
    ui->action2Tone->setChecked(true);
}

void DMMainWindow::on_actionFLDigi_triggered()
{
    closeAllEngines();
    fldigiFrame = new FLDigiFrame(this, ui->rxChars, ui->sendEdit);
}
void DMMainWindow::on_actionExit_triggered()
{
    close();
}
void DMMainWindow::doCloseEvent()
{
    closeAllEngines();


    LogTimer.stop();

    // and tidy up all loose ends

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());


    trace("Minos Data Modes App Closing");

}

void DMMainWindow::on_sendButton_clicked()
{
    QString data = ui->sendEdit->text().trimmed();
    if (mmvariFrame)
    {
        mmvariFrame->sendCharacters(data);
    }
    if (mmttyFrame)
    {
        mmttyFrame->sendCharacters(data);
    }
}

