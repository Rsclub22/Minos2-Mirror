#include "base_pch.h"
#include <QScrollBar>
#include "StartConfig.h"

#include "ConfigFile.h"
#include "ConfigElementFrame.h"
#include "delayedaction.h"

#include "ui_StartConfig.h"

StartConfig::StartConfig(QWidget *parent, bool showAutoStart) :
    QDialog(parent),
    ui(new Ui::StartConfig)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("startConfig/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->autoStartCheckBox->setVisible(showAutoStart);

    QVBoxLayout *vbl = new QVBoxLayout(ui->scrollAreaWidgetContents);
    vbl->setMargin(1);
    ui->scrollAreaWidgetContents->setLayout(vbl);

    elementFrames.clear();
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();

//    int offset = 0;
    for (auto const &c:  minosConfig->elelist)
    {
        if (c->deleted)
            continue;

        ConfigElementFrame *cef = new ConfigElementFrame(false);

        vbl->addWidget(cef);

        cef->setElement(c);
        elementFrames.append(cef);
    }
    ui->StationIdEdit->setText(minosConfig->getThisServerName());
    ui->autoStartCheckBox->setChecked(minosConfig->getAutoStart());

    QString reqErrs = MinosConfig::getMinosConfig() ->checkConfig();

    if (!reqErrs.isEmpty())
    {
        mShowMessage(reqErrs, this);
    }
    formShowTimer.setSingleShot(true);
    connect(&formShowTimer, SIGNAL(timeout()), this, SLOT(on_formShown()));
    formShowTimer.start(100);

    checkEnabled();

    connect(&runTimer, SIGNAL(timeout()), this, SLOT(checkEnabled()));
    runTimer.start(1000);
}
void StartConfig::on_formShown()
{
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();
    if (minosConfig->elelist.size() == 0 )
    {
        // configure a new server
        on_newElementButton_clicked();

        QSharedPointer<RunConfigElement> c(new RunConfigElement());
        c->runType = RunLocal;
        c->appType = "Server";
        c->rEnabled = true;
        elementFrames[0]->setElement(c);
        elementFrames[0]->on_appTypeCombo_currentIndexChanged("Server");

        // and start the next one
        on_newElementButton_clicked();

    }
}
StartConfig::~StartConfig()
{
    delete ui;
}

void StartConfig::checkEnabled()
{
    bool running = MinosConfig::getMinosConfig() ->anyRunning();

    ui->StopButton->setEnabled(running);
    ui->newElementButton->setEnabled(!running);
    ui->SaveCloseButton->setEnabled(!running);
    ui->StartButton->setEnabled(!running);
    ui->SetButton->setEnabled(!running);
    ui->StationIdEdit->setEnabled(!running);
    for (auto const &e: elementFrames)
    {
        e->setEnabled(!running);
    }
}
void StartConfig::reject()
{
    bool running = MinosConfig::getMinosConfig() ->anyRunning();
    if (!running)
        MinosConfig::getMinosConfig() ->reset();
    QDialog::reject();
}
void StartConfig::accept()
{
    QDialog::accept();
}
void StartConfig::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("startConfig/geometry", saveGeometry());
    QDialog::moveEvent(event);
}
void StartConfig::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("startConfig/geometry", saveGeometry());
    QDialog::resizeEvent(event);
}
void StartConfig::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("startConfig/geometry", saveGeometry());
    }
}

void StartConfig::setup(bool started)
{
    ui->StartButton->setEnabled(!started);
    ui->StopButton->setVisible(false);
}
void StartConfig::on_StartButton_clicked()
{
    copyFromScreen();
    QString reqErrs = MinosConfig::getMinosConfig() ->checkConfig();

    if (reqErrs.isEmpty())
    {
        saveAll();
        MinosConfig::getMinosConfig() ->start();
    }
    else
    {
        mShowMessage(reqErrs, this);
    }
    checkEnabled();
}

void StartConfig::on_StopButton_clicked()
{
    MinosConfig::getMinosConfig() ->askStop();
    MinosConfig::getMinosConfig() ->forceStop();
    checkEnabled();
}

void StartConfig::on_autoStartCheckBox_clicked()
{
    MinosConfig::getMinosConfig() ->setAutoStart(ui->autoStartCheckBox->isChecked());
}

void StartConfig::on_SetButton_clicked()
{
    QString coh = ui->StationIdEdit->text();
    MinosConfig::getMinosConfig() ->setThisServerName( coh );
}

void StartConfig::copyFromScreen()
{
    for (auto const &e: elementFrames)
    {
        e->saveElement();
    }

    on_SetButton_clicked();
    on_autoStartCheckBox_clicked();
}
void StartConfig::saveAll()
{
    MinosConfig::getMinosConfig()->saveAll();    // which clears the config file before saving
}
void StartConfig::on_SaveCloseButton_clicked()
{
    copyFromScreen();

    QString reqErrs = MinosConfig::getMinosConfig() ->checkConfig();

    if (reqErrs.isEmpty())
    {
        saveAll();
        accept();
    }
    else
    {
        mShowMessage(reqErrs, this);
    }

}
void StartConfig::on_CancelButton_clicked()
{
    reject();
}

void StartConfig::on_newElementButton_clicked()
{
    // Create new element
    ConfigElementFrame *cef = new ConfigElementFrame(true); // mark as new element

    // set alternating background
/*
    if (elementFrames.size()%2)
    {
        cef->setStyleSheet("QFrame { background-color: lightBlue; }");
    }
    else
    {
        cef->setStyleSheet("QFrame { background-color: white; }");
    }
    cef->fixComboStyle();
*/
    ui->scrollAreaWidgetContents->layout()->addWidget(cef);

    QSharedPointer<RunConfigElement> c = QSharedPointer<RunConfigElement> (new RunConfigElement);
    c->runType = RunLocal;
    c->appType = MinosConfig::tr(MinosConfig::appNone);
    c->rEnabled = true;

    cef->setElement(c);
    elementFrames.append(cef);

    update();

    delayedAction(this, [=]()
    {
        // NB a lambda function
        ui->elementScrollArea->verticalScrollBar()->setValue(ui->elementScrollArea->verticalScrollBar()->maximum());
    }
    );

    cef->setNameFocus();
}
