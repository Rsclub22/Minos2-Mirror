#include <QFontDialog>
#include <QFont>
#include <QStatusBar>
#include <QSettings>

#include "QtUtils.h"
#include "managehamlib.h"
#include "regsettings.h"
#include "MTrace.h"

#include "MShowMessageDlg.h"
#include "ServerEvent.h"
#include "StartConfig.h"
#include "waitcursor.h"
#include "AppStartup.h"
#include "LogEvents.h"
#include "ConfigFile.h"
#include "StartConfigManager.h"
#include "AsMainWindow.h"
#include "ui_AsMainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createCloseEvent();
    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    MinosConfig *mconfig = MinosConfig::getMinosConfig();

    if (mconfig ->getAutoStart())
    {
        connect(&startTimer, &QTimer::timeout, this, &MainWindow::startTimer_Timeout);
        startTimer.start(100);
    }

    sbLabel = new QLabel( "" );
    statusBar() ->addWidget( sbLabel, 6 );

    sbLabel->setText(mconfig->getCurrConfig().configName);

    connect(MinosConfig::getMinosConfig(), &MinosConfig::stdOutLine, this, &MainWindow::on_stdOutLine);

    ExitAction = newAction(QT_TR_NOOP("E&xit Minos Application Starter"), ui->menuFile, &MainWindow::ExitActionExecute);
    FontEditAcceptAction = newAction(QT_TR_NOOP("Select &Font..."), ui->menuTools, &MainWindow::FontEditAcceptActionExecute);
    ManageHamlibAction = newAction(QT_TR_NOOP("Manage &Hamlib..."), ui->menuTools, &MainWindow::ManageHamlibActionExecute);
    languagesMenu = newMenu(ui->menuTools, QT_TR_NOOP("Select &Language"));

    QString currentLang = getCurrentLanguage();

    QVector<Translation> languages = getLanguages();
    for(auto const &l: QASCONST(languages))
    {
        QAction *act =  new QAction(this);
        act->setText(l.dispName);

        connect(act, &QAction::triggered,
                this, &MainWindow::LanguageAcceptActionExecute);
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
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QMainWindow::moveEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QMainWindow::resizeEvent(event);
}
void MainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue("geometry", saveGeometry());
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
QAction *MainWindow::newAction( const char *text, QMenu *m, void (MainWindow::*slotparam)() )
{
    QAction * newAct = new QAction( tr(text), this );
    m->addAction( newAct );
    actionList[newAct] = text;
    connect( newAct, &QAction::triggered, this, slotparam );
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
        RegSettings settings;
        settings.getSettings().remove( "font");
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

            RegSettings settings;
            settings.getSettings().setValue( "font", font() );

            bool routerRunning = checkRouterReady();
            if (routerRunning)
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
        bool routerRunning = checkRouterReady();

        if (lastLanguageSelected)
            lastLanguageSelected->setChecked(false);
        action->setChecked(true);
        lastLanguageSelected = action;
        QString selText = action->text();

        QVector<Translation> languages = getLanguages();
        for(auto const &l: QASCONST(languages))
        {
            if (l.dispName == selText)
            {
                switchTranslation(l.code);
                break;
            }
        }
        if (routerRunning)
        {
            MinosConfig::getMinosConfig() ->bounce();
        }
    }
}

void MainWindow::ManageHamlibActionExecute()
{
    ManageHamlib mhl(this);
    mhl.exec();
}

void MainWindow::on_appsButton_clicked()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    StartConfig configBox( this, true, minosConfig->getCurrConfig().configName);
    configBox.exec();

    minosConfig = MinosConfig::getMinosConfig();
    sbLabel->setText(minosConfig->getCurrConfig().configName);

}

void MainWindow::on_appSelectButton_clicked()
{
    StartConfigManager manageApps( this, true);   // when managing sets, include autostart
    manageApps.exec();
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    sbLabel->setText(minosConfig->getCurrConfig().configName);
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
                                   this,
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
                                   this,
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
                                   this,
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
                                   this,
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


void MainWindow::on_startAppsButton_clicked()
{
    MinosConfig::getMinosConfig()->start();
}

