#include "base_pch.h"
#include <QFontDialog>
#include <QFont>
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

    ExitAction = newAction(QT_TR_NOOP("E&xit Minos Application Starter"), ui->menuFile, SLOT(ExitActionExecute()));
    FontEditAcceptAction = newAction(QT_TR_NOOP("Select &Font..."), ui->menuTools, SLOT(FontEditAcceptActionExecute()));
    languagesMenu = newMenu(ui->menuTools, QT_TR_NOOP("Select &Language"));

    QString currentLang = getCurrentLanguage();

    QVector<Translation> languages = getLanguages();
    for(auto const &l: languages)
    {
        QAction *act =  new QAction(this);
        act->setText(l.dispName);

        connect(act, SIGNAL(triggered()),
                this, SLOT(LanguageAcceptActionExecute()));
        act->setCheckable(true);

        languagesMenu->addAction(act);

        if (l.code == currentLang)
        {
            act->setChecked(true);
            lastLanguageSelected = act;
        }

    }


}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    MinosConfig::getMinosConfig()->askStop();
    MinosConfig::getMinosConfig()->forceStop();

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
    if (e->type() == QEvent::LanguageChange)
    {
        // when language changes force a complete rebuild
        TWaitCursor wc(this);

        for(QMap<QMenu *, const char *>::iterator i = menuList.begin(); i != menuList.end(); i++)
        {
            i.key()->setTitle(tr(i.value()));
        }
        for(QMap<QAction *, const char *>::iterator i = actionList.begin(); i != actionList.end(); i++)
        {
            i.key()->setText(tr(i.value()));
        }
        ui->retranslateUi(this);
        setWindowTitle(tr("Start Minos Apps"));

    }
}
QMenu *MainWindow::newMenu(QMenu *m, const char *text)
{
    QMenu *menu = m->addMenu(tr(text));
    menuList[menu] = text;
    return menu;
}
QAction *MainWindow::newAction( const char *text, QMenu *m, const char *atype )
{
    QAction * newAct = new QAction( tr(text), this );
    m->addAction( newAct );
    actionList[newAct] = text;
    connect( newAct, SIGNAL( triggered() ), this, atype );
    return newAct;
}
void MainWindow::ExitActionExecute()
{
    trace("ExitActionExecute");
    close();
}
void MainWindow::FontEditAcceptActionExecute()
{
    QString qpa = qgetenv("QT_QPA_PLATFORMTHEME");
    if (qpa.compare("qt5ct", Qt::CaseInsensitive) == 0)
    {
        mShowMessage(tr("Font setting will not work while the QT_QPA_PLATFORMTHEME environment variable is set to qt5ct"), this);
        QSettings settings;
        settings.remove( "font");
    }
    else
    {
        QFont f = font();
        bool ok;
        f = QFontDialog::getFont( &ok, f );
        if (ok)
        {
            QApplication::setFont( f );

            for(auto &widget: QApplication::allWidgets() )
            {
                widget->setFont(f);
                widget->update();
            }

            QSettings settings;
            settings.setValue( "font", font() );

            QString fs = f.toString();

            bool serverRunning = checkServerReady();
            if (serverRunning)
            {
                MinosConfig::getMinosConfig() ->bounce();
            }
        }
    }
}
void MainWindow::LanguageAcceptActionExecute()
{
    TWaitCursor wc(this);
    QAction *action = qobject_cast<QAction *>(sender());

    if (action)
    {
        bool serverRunning = checkServerReady();

        if (lastLanguageSelected)
            lastLanguageSelected->setChecked(false);
        action->setChecked(true);
        lastLanguageSelected = action;
        QString selText = action->text();

        QVector<Translation> languages = getLanguages();
        for(auto const &l: languages)
        {
            if (l.dispName == selText)
            {
                switchTranslation(l.code);
                break;
            }
        }
        if (serverRunning)
        {
            MinosConfig::getMinosConfig() ->bounce();
        }
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
    if (l.size() == 3)  // we have actually split so we have just one command
    {
        if (l[1] == "RestartApps")
        {
            m_connection = connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped,
                                   [this]{
                                        this->disconnect(m_connection);
                                        m_connection =  QMetaObject::Connection();
                                        MinosConfig::getMinosConfig()->start();
                                    });
            MinosConfig::getMinosConfig()->askStop();
            MinosConfig::getMinosConfig()->forceStop();
        }
        else if (l[1] == "CloseApps")
        {
            m_connection = connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped,
                                   [this]{
                                        this->disconnect(m_connection);
                                        m_connection =  QMetaObject::Connection();
                                        this->close();
                                    });
            MinosConfig::getMinosConfig()->askStop();
            MinosConfig::getMinosConfig()->forceStop();
        }
        else if (l[1] == "RestartOS")
        {
            m_connection = connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped,
                                   [this]{
                                        this->disconnect(m_connection);
                                        m_connection =  QMetaObject::Connection();
                                        system("systemctl reboot");
                                        this->close();
                                    });
            MinosConfig::getMinosConfig()->askStop();
            MinosConfig::getMinosConfig()->forceStop();
        }
        else if (l[1] == "CloseOS")
        {
            m_connection = connect(MinosConfig::getMinosConfig(), &MinosConfig::allStopped,
                                   [this]{
                                        this->disconnect(m_connection);
                                        m_connection =  QMetaObject::Connection();
                                        system("systemctl poweroff");
                                        this->close();
                                    });
            MinosConfig::getMinosConfig()->askStop();
            MinosConfig::getMinosConfig()->forceStop();
        }
    }
}

