#include "bandmapmainwindow.h"
#include "cutils.h"
#include "rigcontrol.h"
#include "bandmap.h"
#include "freqdial.h"
#include "textmarker.h"
#include "bmsetupdialog.h"
#include "ui_bandmapmainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QDebug>

BandMapMainWindow::BandMapMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BandMapMainWindow)
{
    ui->setupUi(this);
    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    radio = new RigControl();
    selectRig = new BMSetupDialog(radio);
    timer = new QTimer(this);
    status = new QLabel;
    selectRadio = new QComboBox;

    bandmapScene = new QGraphicsScene(this);
    ui->bandmapView->setScene(bandmapScene);
    ui->bandmapView->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    bandmap = new Bandmap();
    dial = new FreqDial();

//    ui->bandmapView->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    bandmapScene->addItem(dial);





    radio->set_serialConnected(false);
    initActionsConnections();

    selectRadio = ui->selectRadioBox;

    initSelectRadioBox();
    selectRig->readCurrentRadio();
    selectRadio->setCurrentIndex(selectRadio->findText(selectRig->currentRadio.radioName));

    setPolltime(250);

    openRadio();





    callSignText = new TextMarker(80,150, "G8FKH", Qt::red);
    bandmapScene->addItem(callSignText);
    bandmapScene->addItem(new TextMarker(80,160, "M0ICR", Qt::yellow));
    bandmapScene->addItem(new TextMarker(80,200, "GD8EXI", Qt::black));
    bandmapScene->addItem(new TextMarker(80,250, "M0SAT", Qt::blue));
//    dial->setPos(-100,-50);
    bandmap->addCallsignMarker(144000000.00, "G8FKH", "12:30", 0, 120);
    bandmap->addCallsignMarker(144000000.00, "G8LZE", "12:50", 0, 200);
    bandmap->addCallsignMarker(144000000.00, "M0ICR", "12:55", 0, 140);






}

BandMapMainWindow::~BandMapMainWindow()
{
    delete ui;
}

void BandMapMainWindow::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(false);
}

void BandMapMainWindow::initActionsConnections()
{
    connect(ui->selectRadioBox, SIGNAL(activated(int)), this, SLOT(upDateRadio()));

    connect(ui->actionSetup_Radios, SIGNAL(triggered()), selectRig, SLOT(show()));

    connect(timer, SIGNAL(timeout()), this, SLOT(getFrequency()));
    connect(radio, SIGNAL(frequency_updated(double)), this, SLOT(updateFreq(const double)));
    connect(this, SIGNAL(frequency_updated(double)), this, SLOT(displayFreq(const double)));
    connect(this, SIGNAL(frequency_updated(double)), this, SLOT(drawDial(double)));
    //connect(ui->actionClear, SIGNAL(triggered()), console, SLOT(clear()));
    //connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    //connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}



void BandMapMainWindow::initSelectRadioBox()
{
    selectRadio->addItem("");
    for (int i= 0; i < NUM_RADIOS; i++)
    {
        selectRadio->addItem(selectRig->availRadios[i].radioName);
    }
}




void BandMapMainWindow::upDateRadio()
{
    int radioIndex = ui->selectRadioBox->currentIndex();
    if (radioIndex > 0)
    {
        radioIndex -= 1;
        if (selectRig->availRadios[radioIndex].radioModelNumber == 0)
        {
            closeRadio();
            QMessageBox::critical(this, tr("Radio Error"), "Please select a radio model");
            return;
        }
        selectRig->currentRadio.radioName = selectRig->availRadios[radioIndex].radioName;
        selectRig->currentRadio.radioMfg_Name = selectRig->availRadios[radioIndex].radioMfg_Name;
        selectRig->currentRadio.radioModel = selectRig->availRadios[radioIndex].radioModel;
        selectRig->currentRadio.radioModelNumber = selectRig->availRadios[radioIndex].radioModelNumber;
        selectRig->currentRadio.civAddress = selectRig->availRadios[radioIndex].civAddress;
        selectRig->currentRadio.comport = selectRig->availRadios[radioIndex].comport;
        selectRig->currentRadio.baudrate = selectRig->availRadios[radioIndex].baudrate;
        selectRig->currentRadio.databits = selectRig->availRadios[radioIndex].databits;
        selectRig->currentRadio.stopbits = selectRig->availRadios[radioIndex].stopbits;
        selectRig->currentRadio.parity = selectRig->availRadios[radioIndex].parity;
        selectRig->currentRadio.handshake = selectRig->availRadios[radioIndex].handshake;
        selectRig->saveCurrentRadio();
        if (radio->get_serialConnected())
       {
                closeRadio();
       }
       openRadio();

    }


}

void BandMapMainWindow::openRadio()
{

    int retCode = 0;

    if (selectRig->currentRadio.radioName == "")
    {
        showStatusMessage("Please select a Radio");
        return;
    }
    if(selectRig->currentRadio.comport == "")
    {
        showStatusMessage("Please select a Comport");
        return;
    }
    if (selectRig->currentRadio.radioModel == "")
    {
        showStatusMessage("Please select a radio model");
        return;
    }



    scatParams p = selectRig->getCurrentRadio();

    retCode = radio->init(selectRig->currentRadio);
    if (retCode < 0)
    {
        hamlibError(retCode);
    }
    if (radio->get_serialConnected())
    {

        timer->start(pollTime);             // start timer to send message to controller
        showStatusMessage(tr("Connected to Radio: %1 - %2, %3, %4, %5, %6, %7, %8")
                              .arg(p.radioName).arg(p.radioModel).arg(p.comport).arg(p.baudrate).arg(p.databits)
                              .arg(p.stopbits).arg(radio->getParityCodeNames()[p.parity]).arg(radio->getHandShakeNames()[p.handshake]));

    }
    else
    {
//        QMessageBox::critical(this, tr("Error"), serial->errorString());
        showStatusMessage(tr("Radio Open error"));
    }


}

void BandMapMainWindow::closeRadio()
{
    radio->closeRig();
    showStatusMessage(tr("Disconnected"));
}


void BandMapMainWindow::setPolltime(int interval)
{
    pollTime = interval;
}

int BandMapMainWindow::getPolltime()
{

    return pollTime;
}


void BandMapMainWindow::getFrequency()
{
    if (radio->get_serialConnected())
    {
        int retCode = 0;
        retCode = radio->getFrequency();
        if (retCode < 0)
        {
            hamlibError(retCode);
        }
    }
}


void BandMapMainWindow::updateFreq(double frequency)
{
    if (almost_equal(curFreq, frequency, 2))
    {
        return;
    }
    else
    {
        curFreq = frequency;
    }
    emit frequency_updated(frequency);
}



void BandMapMainWindow::displayFreq(double frequency)
{

    ui->radioFreq->setText(convertStringFreq(frequency));
}




void BandMapMainWindow::drawDial(double frequency)
{


    if (!almost_equal(frequency, dial->getCurFreq(), 2))
    {
        dial->setCurFreq(frequency);
        dial->setCurHeight(ui->bandmapView->height());
        dial->update();
    }

}




QString BandMapMainWindow::convertStringFreq(double frequency)
{
    double freq = frequency;
    sfreq = "";
    qint32 f = 0;
    f = static_cast<qint32>(freq/1000000);
    sfreq = QString::number(f) + ".";
    freq = freq - (f*1000000);
    f = static_cast<qint32>(freq/1000);
    if (f < 10)
    {
        sfreq = sfreq + "00";
    }
    else if (f < 100)
    {
        sfreq = sfreq + "0";
    }
    sfreq = sfreq + QString::number(f) + ".";
    freq = freq - (f*1000);
    if (freq < 10)
    {
        sfreq = sfreq + "00";
    }
    else if (freq < 100)
    {
        sfreq = sfreq + "0";
    }
    sfreq = sfreq + QString::number(freq);

    return sfreq;
}




void BandMapMainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}






void BandMapMainWindow::hamlibError(int errorCode)
{
    int errCode = errorCode;
    if ( errCode >= 0)
    {
        return;
    }

    errCode *= -1;
    QString errorMsg = radio->gethamlibErrorMsg(errCode);

    QMessageBox::critical(this, "hamlib Error", QString::number(errCode) + " - " + errorMsg);


}





void BandMapMainWindow::resizeEvent(QResizeEvent *event)
{
    mapViewHeight = ui->bandmapView->height() - 2;
    if (dial->getCurHeight() != mapViewHeight)
    {
        dial->changeBoundingRect(mapViewHeight);
        bandmapScene->setSceneRect(bandmapScene->itemsBoundingRect());
    }
    qDebug() << "view height" << mapViewHeight;
    qDebug() << "dial height" << dial->boundingRect().height();

    dial->setCurFreq(curFreq);
    //dial->setCurHeight(mapViewHeight);
    dial->update();

    QWidget::resizeEvent(event);
}



void BandMapMainWindow::paintEvent(QPaintEvent  * /*event */)
{

}
