#include "base_pch.h"
#include "ConfigFile.h"
#include "StartConfig.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createCloseEvent();
    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    if (MinosConfig::getMinosConfig() ->getAutoStart())
    {
        connect(&startTimer, SIGNAL(timeout()), this, SLOT(startTimer_Timeout()));
        startTimer.start(100);
    }

    connect(MinosConfig::getMinosConfig(), SIGNAL(stdOutLine(QString)), this, SLOT(on_stdOutLine(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}
void MainWindow::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QMainWindow::moveEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QMainWindow::resizeEvent(event);
}
void MainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}

void MainWindow::on_appsButton_clicked()
{

    StartConfig startConfig(this, true);
    startConfig.exec();
}

void MainWindow::on_closeButton_clicked()
{
    close();
}
void MainWindow::startTimer_Timeout()
{
    startTimer.stop();
    start();
}

void MainWindow::start()
{
    MinosConfig::getMinosConfig() ->start();
}

void MainWindow::on_stdOutLine(QString line)
{
    QStringList l = line.split("!!");
    if (l.size() == 1 && l[0] != line)  // we have actually split so we have just one command
    {
        QMetaObject::Connection m_connection;
        if (line == "!!RestartApps!!")
        {
            MinosConfig::getMinosConfig()->stop();
            //connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped, MinosConfig::getMinosConfig(), &MinosConfig::start);
            m_connection = connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped,
                                   [&m_connection]{
                                        disconnect(m_connection);
                                        MinosConfig::getMinosConfig()->start();
                                    });
        }
        else if (line == "!!CloseApps!!")
        {
            MinosConfig::getMinosConfig()->stop();
            m_connection = connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped,
                                   [this, &m_connection]{
                                        disconnect(m_connection);
                                        this->close();
                                    });
        }
        else if (line == "!!RestartOS!!")
        {
            MinosConfig::getMinosConfig()->stop();
            m_connection = connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped,
                                   [this, &m_connection]{
                                        disconnect(m_connection);
                                        system("systemctl reboot");
                                        this->close();
                                    });
        }
        else if (line == "!!CloseOS!!")
        {
            MinosConfig::getMinosConfig()->stop();
            m_connection = connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped,
                                   [this, &m_connection]{
                                        disconnect(m_connection);
                                        system("systemctl poweroff");
                                        this->close();
                                    });
        }
    }
}

