#include "base_pch.h"
#include <QScrollBar>
#include "StartConfig.h"

#include "ConfigFile.h"
#include "ConfigElementFrame.h"

#include "ui_StartConfig.h"

StartConfig::StartConfig(QWidget *parent, bool showAutoStart) :
    QDialog(parent),
    ui(new Ui::StartConfig)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("startConfigGeometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    ui->autoStartCheckBox->setVisible(showAutoStart);

    QVBoxLayout *vbl = new QVBoxLayout(ui->scrollAreaWidgetContents);
    vbl->setMargin(1);
    ui->scrollAreaWidgetContents->setLayout(vbl);

    elementFrames.clear();
    MinosConfig *minosConfig = MinosConfig::getMinosConfig();

//    int offset = 0;
    for (int i = 0; i <  minosConfig->elelist.size(); i++)
    {
        QSharedPointer<RunConfigElement> c = minosConfig->elelist[i];
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

    checkEnabled();

    connect(&runTimer, SIGNAL(timeout()), this, SLOT(checkEnabled()));
    runTimer.start(1000);
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
    for (int i = 0; i < elementFrames.size(); i++)
    {
        elementFrames[i]->setEnabled(!running);
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
    settings.setValue("startConfigGeometry", saveGeometry());
    QDialog::moveEvent(event);
}
void StartConfig::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("startConfigGeometry", saveGeometry());
    QDialog::resizeEvent(event);
}
void StartConfig::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("startConfigGeometry", saveGeometry());
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
    MinosConfig::getMinosConfig() ->stop();
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
    for (int i = 0; i < elementFrames.size(); i++)
    {
        elementFrames[i]->saveElement();
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
    c->appType = "None";
    c->rEnabled = true;

    cef->setElement(c);
    elementFrames.append(cef);

    repaint();

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, [=]()
    {
        // NB a lambda function
        ui->elementScrollArea->verticalScrollBar()->setValue(ui->elementScrollArea->verticalScrollBar()->maximum());
        timer->deleteLater();
    }
    );

    timer->start(100);
    cef->setNameFocus();
}
