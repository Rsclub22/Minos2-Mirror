#include <QMenuBar>
#include <QMenu>
#include <QSplitter>
#include "MTrace.h"
#include "cutils.h"
#include "rxbuffer.h"
#include "dmmainwindow.h"
#include "MMVARIFrame.h"
#include "ui_MMVARIFrame.h"

// we already report on the TX/RX frequencies!
// either pallette of sensitivity are rubbish - not seeoing signals
MMVARIFrame::MMVARIFrame(QWidget *parent, QFrame *cwl,
                         QLineEdit *sendEdit,
                         int inId, int outId) :
    QFrame(parent),
    ui(new Ui::MMVARIFrame),
    sendEdit(sendEdit),
    pframe(cwl)
{
    ui->setupUi(this);

    connect(mainWindow, &DMMainWindow::sendCharacters, this, &MMVARIFrame::onSendCharacters);
    connect(mainWindow, &DMMainWindow::rigModeFreq, this, &MMVARIFrame::onRigModeFreq);

    // NB - the OCX MUST be alongside the executable, not in the"runtime" directory (if that is different)
    // so e.g. when debugging, in
    // C:\Projects\build-maiatop-Desktop_Qt_5_15_2_MinGW_32_bit-Debug\mmvariTest\debug

    // We build in a manifest to allow Windows to find the correct OCX

    mmvari = new MMVARILib::MMVARI();
    mmvari->setControl("{9C0D49DD-5C05-456D-916B-98C4CF63172F}");

    mmview = new MMVARILib::XMMVView(this);
    mmview->setControl("{702CBF07-C159-44F2-B8A6-DF8EA1001E08}");

    mmview2 = new MMVARILib::XMMVView(this);
    mmview2->setControl("{702CBF07-C159-44F2-B8A6-DF8EA1001E08}");

    mmlevel = new MMVARILib::XMMVLvl(this);
    mmlevel->setControl("{438EF93A-939D-4B6B-93A7-DF09049B8514}");

    mmvariVb = new QVBoxLayout(this);

    //====================================================================
    // N1MM also has BPF, ATC, FFT, Multi-Channel RX menus
    // Do we need to have anything here?

    QMenuBar *mmbar = new QMenuBar(this);
    QMenu * bpfMenu = new QMenu("BPF");
    QMenu * atcMenu = new QMenu("ATC");
    QMenu * fftMenu = new QMenu("FFT");
    QMenu * mcMenu = new QMenu("Multi-Channel RX");

    mmbar->addMenu(bpfMenu);
    mmbar->addMenu(atcMenu);
    mmbar->addMenu(fftMenu);
    mmbar->addMenu(mcMenu);

    newCheckableAction("On", atcMenu, &MMVARIFrame::onATC);
    onATC(mmvari->bATC());

    mmvariVb->addWidget(mmbar);

    //====================================================================
    // we need a row of buttons here...

    // to match N1MM+ we need buttons

    // TX, RX, AFC, NET, ALIGN, mode combo, speed combo, TX Carrier, RX Carrier, s/n indicator

    QHBoxLayout *mmvariButtons = new QHBoxLayout();

    txButton = new QPushButton();
    txButton->setCheckable(true);
    txButton->setText("TX");
    mmvariButtons->addWidget(txButton);
    connect(txButton, &QPushButton::clicked, this, &MMVARIFrame::txButtonClicked);

    rxButton = new QPushButton();
    rxButton->setCheckable(true);
    rxButton->setText("RX");
    rxButton->setChecked(true);
    mmvariButtons->addWidget(rxButton);
    connect(rxButton, &QPushButton::clicked, this, &MMVARIFrame::rxButtonClicked);

    afcButton = new QPushButton();
    afcButton->setCheckable(true);
    afcButton->setText("AFC");
    mmvariButtons->addWidget(afcButton);
    connect(afcButton, &QPushButton::clicked, this, &MMVARIFrame::afcButtonClicked);

    netButton = new QPushButton();
    netButton->setCheckable(true);
    netButton->setText("NET");
    mmvariButtons->addWidget(netButton);
    connect(netButton, &QPushButton::clicked, this, &MMVARIFrame::netButtonClicked);

    alignButton = new QPushButton();
    alignButton->setText("Align");
    mmvariButtons->addWidget(alignButton);
    connect(alignButton, &QPushButton::clicked, this, &MMVARIFrame::alignButtonClicked);


    modeCombo = new QComboBox();
    mmvariButtons->addWidget(modeCombo);

    speedCombo = new QComboBox(this);
    mmvariButtons->addWidget(speedCombo);

    rxCarrier = new QLabel();
    mmvariButtons->addWidget(rxCarrier);
    txCarrier = new QLabel();
    mmvariButtons->addWidget(txCarrier);
    snLabel = new QLabel();
    mmvariButtons->addWidget(snLabel);

    mmvariButtons->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    modeCombo->clear();
    modeCombo->addItem ("bpsk");
    modeCombo->addItem ("rtty-L");
    modeCombo->addItem ("rtty-U");
    connect(modeCombo, &QComboBox::currentTextChanged, this, &MMVARIFrame::onModeComboChanged);

    speedCombo->clear();

    connect(speedCombo, &QComboBox::currentTextChanged, this, &MMVARIFrame::onSpeedComboChanged);
    mmvariVb->addLayout(mmvariButtons);

    modeCombo->setCurrentIndex(0);
    onModeComboChanged(speedCombo->currentText());


    //====================================================================
    // and finally the spectrum and level controls
    mmvariHb = new QHBoxLayout();
    mmvariHb->addWidget(mmlevel, 1);

    mvb = new QVBoxLayout();
    mvb->addWidget(mmview);
    mvb->addWidget(mmview2);
    mmvariHb->addLayout(mvb, 20);

    mmvariVb->addLayout(mmvariHb);

    //    wType As Integer
//    ~~~~~~~~~~~~~~~~
//    This property defines the display content.
//        0=viewtypeFFT		Specturm
//        1=viewtypeWATER		Waterfall
//        2=viewtypeWAVE		Waveform
//        3=viewtypeFREQERR	AFC level meter
//        4=viewtypeSMALLWATER	Waterfall without scale
    mmview->setWType(0);
    mmview2->setWType(1);

    mmvariWnd = mmvari->dwHandle();

    // COM/OLE exceptions get signaled
    connect (&*mmvari, SIGNAL (exception(int,QString,QString,QString)), this, SLOT (onHandleMMVARICOMException(int,QString,QString,QString)));
    connect (&*mmvari, SIGNAL (signal(const QString&,int,void*)), this, SLOT (onMMVARISignal(const QString&,int,void*)));
    connect (&*mmvari, SIGNAL (propertyChanged(const QString&)), this, SLOT (onMMVARIPropertyChanged(const QString&)));

    connect (&*mmview, SIGNAL (exception(int,QString,QString,QString)), this, SLOT (onHandleXMMVIEWCOMException(int,QString,QString,QString)));
    connect (&*mmview, SIGNAL (signal(const QString&,int,void*)), this, SLOT (onXMMVIEWSignal(const QString&,int,void*)));
    connect (&*mmview, SIGNAL (propertyChanged(const QString&)), this, SLOT (onXMMVIEWPropertyChanged(const QString&)));

    // event signals have to be old style - linkage is done at run time
    connect (&*mmvari, SIGNAL (OnMode(int,int,QString)), this, SLOT (OnMode(int,int,QString)));
    connect (&*mmvari, SIGNAL (OnDrawFFT(int,int&)), this, SLOT (OnDrawFFT(int,int&)));
    connect (&*mmvari, SIGNAL (OnTxCarrier(int)), this, SLOT (OnTxCarrier(int)));
    connect (&*mmvari, SIGNAL (OnRxCarrier(int,int)), this, SLOT (OnRxCarrier(int,int)));
    connect (&*mmvari, SIGNAL (OnSpeed(int,double)), this, SLOT (OnSpeed(int,double)));
    connect (&*mmvari, SIGNAL (OnRxChar(int,QString,int)), this, SLOT (OnRxChar(int,QString,int)));
    connect (&*mmvari, SIGNAL (OnTiming(int,int,int)), this, SLOT (OnTiming(int,int,int)));
    connect (&*mmvari, SIGNAL (OnGetTxChar(int&)), this, SLOT (OnGetTxChar(int&)));
    connect (&*mmvari, SIGNAL (OnDrawWave(int,int&,int&)), this, SLOT (OnDrawWave(int,int&,int&)));
    connect (&*mmvari, SIGNAL (OnPTT(int)), this, SLOT (OnPTT(int)));
    connect (&*mmvari, SIGNAL (OnNET(int)), this, SLOT (OnNET(int)));
    connect (&*mmvari, SIGNAL (OnTxState(int)), this, SLOT (OnTxState(int)));
    connect (&*mmvari, SIGNAL (OnPlayBack(int)), this, SLOT (OnPlayBack(int)));
    connect (&*mmvari, SIGNAL (OnClockAdjust(int&)), this, SLOT (OnClockAdjust(int&)));
    connect (&*mmvari, SIGNAL (OnError(int)), this, SLOT (OnError(int)));


// View and level are all mouse related

    // we need to configure the sound card!
    // -1 is default; for two cards it is (RX,TX)

    QString devs = QString("%1,%2").arg(inId).arg(outId);

    mmvari->setStrSoundID(devs);

    mmvari->setStrMode(0, modeCombo->currentText());
    mmvari->setWCharset(0, mmvari->wDefaultCharset());

    mmview->setWType(MMVARILib::MMVX_VIEWTYPE::viewtypeFFT);
    mmview->setWWidthFreq(3000);    // frequency span in spectrum window

    //*********************
    mmview->setBLSB(true);

    mmvari->setBActive(true);

    cwl->setLayout(mmvariVb);
}

MMVARIFrame::~MMVARIFrame()
{
    // This is complicated as we want to get rid of all controls
    // when we switch engines, and rebuild them when MMVARI is selected again
    if (mmvari)
    {
        mmvari->setBActive(false);

        while (mmvariHb->count())
        {
            QLayoutItem *l = mmvariHb->takeAt(0);
            QWidget *w = l->widget();
            if (w && w == mmlevel)
            {
                mmlevel->clear();
                mmlevel = nullptr;
            }
        }
        while (mvb->count())
        {
            QLayoutItem *l = mvb->takeAt(0);
            QWidget *w = l->widget();
            if (w && w == mmview)
            {
                mmview->clear();
                mmview = nullptr;
            }
            else if (w && w == mmview2)
            {
                mmview2->clear();
                mmview2 = nullptr;
            }
        }
        delete mvb;

        mmvari->clear();
        mmvari = nullptr;
    }
    QLayout *l = pframe->layout();


    clearLayout(l);
    delete l;
    delete ui;
}

void MMVARIFrame::onSendCharacters(QString data, int c)
{
    sendCharacters(data, c);
}

void MMVARIFrame::onRigModeFreq(QString, Frequency f)
{
    mmview->setDwFreqHz(f.toInt64());  // tranciever frequency

}
void MMVARIFrame::sendCharacters(const QString &sendData, int c)
{
    //    wTxState As Integer (ReadOnly)
    //    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //    This property shows the TX/RX status.
    //        0 - txstateRX		RX
    //        1 - txstateTX		TX
    //        2 - txstateREQRX		TX and waiting for idle
    //        3 - txstateWAIT		Switching to RX (flushing PCM data in the sound buffer)
    //        4 - txstateTONE		Transmitting a single tone

    if (sendData.isEmpty())
    {
        mmvari->setBTX(false);  // stop immediately
    }
    else
    {
        mmvari->setWTxCarrier(c);
        mmvari->setBAddStartCR(true);
        mmvari->setBAddStopCR(true);
        mmvari->SendText(sendData);
        mmvari->setBTX(true);
        mmvari->setBReqRX(true);    // return to RX when buffer empty
    }
}

void MMVARIFrame::sendMode(QString m)
{
    if (m == "PS")
    {
        mmview->setBLSB(false);
        modeCombo->setCurrentText("bpsk");
    }
    if (m == "RY")
    {
        mmview->setBLSB(true);
        modeCombo->setCurrentText("rtty-L");
    }
}
QMenu *MMVARIFrame::newMenu(QMenu *m, const char *text)
{
    QMenu *menu = m->addMenu(tr(text));
    menuList[menu] = text;
    return menu;
}
QAction *MMVARIFrame::newAction(const char *text, QMenu *m, void (MMVARIFrame::*slotparam)() )
{
    QAction * newAct = new QAction( tr(text), this );
    actionList[newAct] = text;
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *MMVARIFrame::newAction(int n, QMenu *m, void (MMVARIFrame::*slotparam)() )
{
    QAction * newAct = new QAction( QString::number(n), this );
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *MMVARIFrame::newCheckableAction( const char *text, QMenu *m, void (MMVARIFrame::*slotparam)(bool) )
{
    QAction * newAct = new QAction( tr(text), this );
    actionList[newAct] = text;
    newAct->setCheckable( true );
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *MMVARIFrame::newCheckableAction(const QString text, QMenu *m, void (MMVARIFrame::*slotparam)(bool) )
{
    QAction * newAct = new QAction( text, this );
    newAct->setCheckable( true );
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}

void MMVARIFrame::onATC(bool checked)
{
    if (mmvari)
    {
        mmvari->setBATC(checked);
    }
}
void MMVARIFrame::onModeComboChanged(const QString &m)
{
    mmvari->setStrMode(0, m);
    speedCombo->clear();

    if (m.contains("rtty"))
    {
        if (m.contains("rtty_L", Qt::CaseInsensitive))
        {
            mmview->setBLSB(true);
        }
        else
        {
            mmview->setBLSB(false);
        }
        speedCombo->addItem("45.45");
        speedCombo->addItem("75");

        speedCombo->setCurrentText("45.45");
    }
    else
    {
        mmview->setBLSB(false);

        speedCombo->addItem("31.25");
        speedCombo->addItem("62.5");

        speedCombo->setCurrentText("62.5");
    }
}
void MMVARIFrame::onSpeedComboChanged(const QString &s)
{
    mmvari->setDblSpeed(0, s.toDouble());
}
void MMVARIFrame::txButtonClicked(bool checked)
{
//    wTxState As Integer (ReadOnly)
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    This property shows the TX/RX status.
//        0 - txstateRX		RX
//        1 - txstateTX		TX
//        2 - txstateREQRX		TX and waiting for idle
//        3 - txstateWAIT		Switching to RX (flushing PCM data in the sound buffer)
//        4 - txstateTONE		Transmitting a single tone

    if (checked)
    {
        mmvari->setBAddStartCR(true);
        mmvari->setBAddStopCR(true);
        mmvari->SendText(sendEdit->text().trimmed());
        mmvari->setBTX(true);
        mmvari->setBReqRX(true);    // return to RX when buffer empty
    }
}
void MMVARIFrame::rxButtonClicked(bool checked)
{
    if (checked)
    {
        mmvari->setBTX(false);
    }
}
void MMVARIFrame::afcButtonClicked(bool checked)
{
    mmvari->setBAFC(0, checked);
}
void MMVARIFrame::netButtonClicked(bool checked)
{
    mmvari->setBNET(checked);
}
void MMVARIFrame::alignButtonClicked(bool /*checked*/)
{
    int rxF = mmvari->wRxCarrier(0);
    mmvari->setWTxCarrier(rxF);
}
void MMVARIFrame::OnMode(int a,int b,QString c)
{
//    OnMode(nIndex As Integer, mIndex As Integer, strMode As String)
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//      nIndex : RX channel #
//      mIndex : Mode number (0-)
//      strMode : Mode name

//    This event occurs when the mode of RX channel(nIndex) is changed. It also occurs just after the RX channel is activated.

    trace( QString("%1: %2 %3 %4").arg("MMVARIFrame::OnMode(int,int,QString)").arg(a).arg(b).arg(c));
    modeCombo->setCurrentText(c);
}
void MMVARIFrame::OnDrawFFT(int ,int& )
{
    if (mmview)
    {
        mmview->Draw(mmvariWnd);
        mmview2->Draw(mmvariWnd);
        mmlevel->Draw(mmvariWnd, 0);

        snLabel->setText(QString("S/N %1 db").arg(mmvari->wSN(0)));

    }
}

void MMVARIFrame::OnTxCarrier(int txc)
{
    txCarrier->setText(QString("Tx %1").arg(txc));
}

void MMVARIFrame::OnRxCarrier(int /*rxChannel*/, int rxc)
{
    rxCarrier->setText(QString("Rx %1").arg(rxc));
    carrier = rxc;
}

void MMVARIFrame::OnSpeed(int /*rxChannel*/, double dblSpeed)
{
//    Speed(nIndex As Integer, dblSpeed As Double)
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//      nIndex : RX channel #
//      dblSpeed : Speed (Baud)

//    This event occurs when the speed of RX channel(nIndex) is changed. It also occurs just after the RX channel is activated.

    QString speed = QString::number(dblSpeed, 'f', QLocale::FloatingPointShortest);


    speedCombo->setCurrentText(speed);
}

void MMVARIFrame::OnRxChar(int /*rxChannel*/, QString strChar, int /*wChar*/)
{
//    OnRxChar(ByVal nIndex As Integer, ByVal strChar As String, ByVal wChar As Integer)
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//      nIndex : RX channel #
//      strChar : Received character (BSTR)
//      wChar : Received character (ANSI or MBCS)

//    This event occurs when a character is received in RX channel(nIndex).
//      You do not have to care about odd delimiters even in MBCS.
//      strChar and wChar are the same data expressed in different formats,
//      so you could use either of them as you like.

    for (auto c:strChar)
    {
        RXChar rxch(c, false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch);
    }
}

void MMVARIFrame::OnTiming(int /*a*/, int /*b*/, int /*c*/)
{
//    OnTiming(nIndex As Integer, dwTiming As Long, wUnit As Integer)
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//      nIndex : RX channel #
//      dwTiming : Timing value
//      wUnit : Unit of timing value (0=PPM, 1=ms multiplied by 100)
//    This event occurs when the RX clock timing is changed in RX chennel(nIndex).
//     In the RTTY mode, the unit of the timing value is ms multiplied by 100.
//    0 means MMVARI is calculating the timing.
//    In all the modes other than RTTY, the unit of the timing value is ppm.

//    traceQString("%1: %2 %3 %4").arg("MMVARIFrame::OnTiming(int, int, int)").arg(a).arg(b).arg(c));
}
void MMVARIFrame::OnGetTxChar(int &/*a*/)
{
//    Asynchronous method
//    ~~~~~~~~~~~~~~~~~~
//    This method transmits characters through the circulate TX buffer equipped in MMVARI.ocx. The buffer can hold up to 2048
//    characters. For this purpose, the following properties and methods are provided:

//        wBufferCount As Integer
//        Function SendText(strText As String) As Integer
//        Function SendCWID(strText As String) As Integer
//        Function SendChar(wChar As Integer) As Integer
//        Function GetSendText() As String

//    For more information, see the property and method sections.

//    Synchronous method
//    ~~~~~~~~~~~~~~~~
//    MMVARI.ocx requests one character on OnGetTxChar event. This method is suited for the scheme
//            in which the user application peeks TX characters one bye one from its TX window and
//            sends them to MMVARI.ocx.

//    * The sample container uses this method for reference.
//    * You could use the asynchronous and synchronous methods at the same time. MMVARI control
//            checks if the TX buffer is empty; if so, it generates OnGetTxChar event to inquire
//            the next character to transmit.

//    You can call SendText method in the OnGetTxChar event handler. In that case, you cannot put any character in wChar.

//        Private Sub MMVARI_OnGetTxChar(wChar As Integer)
//            Call MMVARI.SendText ("Hello")
//        End Sub

//    * The sample container in this package has an example code of this event.

//    * The sound processor has a buffer, and therefore you will receive several OnGetTxChar events before the sound buffer is fulfilled.


//    trace(QString("%1").arg("MMVARIFrame::OnGetTxChar(int&)"));

    // we ignore this; we set tx data on receipt from minos, and return to RX on completion
}
void MMVARIFrame::OnDrawWave(int , int &, int &)
{
    if (mmview)
    {
        //mmview->DrawWave(mmvariWnd);
    }
    if (mmview2)
    {
        //mmview2->DrawWave(mmvariWnd);
    }
}
void MMVARIFrame::OnPTT(int )
{
//    trace(QString("%1: %2").arg("MMVARIFrame::OnPTT(int)").arg(a));
}
void MMVARIFrame::OnNET(int )
{
    //trace(QString("%1: %2").arg("MMVARIFrame::OnNet(int)").arg(a));
}
void MMVARIFrame::OnTxState(int a)
{
    //trace(QString("%1: %2").arg("MMVARIFrame::OnTxState(int)").arg(a));
//    wTxState As Integer (ReadOnly)
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    This property shows the TX/RX status.
//        0 - txstateRX		RX
//        1 - txstateTX		TX
//        2 - txstateREQRX		TX and waiting for idle
//        3 - txstateWAIT		Switching to RX (flushing PCM data in the sound buffer)
//        4 - txstateTONE		Transmitting a single tone

    if (a == 0)
    {
        txButton->setText("TX");
        txButton->setChecked(false);
        rxButton->setChecked(true);

        RXChar rxch('T', true, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch);
        RXChar rxch2('X', false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch2);
        RXChar rxch3(' ', false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch3);

    }
    else if (a == 1)
    {
        txButton->setText("RX");
        txButton->setChecked(true);
        rxButton->setChecked(false);
        RXChar rxch('R', true, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch);
        RXChar rxch2('X', false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch2);
        RXChar rxch3(' ', false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch3);
    }
    else if (a == 2 || a == 3)
    {
        txButton->setText("Wait");
        txButton->setChecked(true);
        rxButton->setChecked(false);
        RXChar rxch('W', true, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch);
        RXChar rxch2('T', false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch2);
        RXChar rxch3(' ', false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch3);
    }
    else if (a == 4)
    {
        txButton->setText("Tone");
        txButton->setChecked(true);
        rxButton->setChecked(false);
        RXChar rxch('T', true, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch);
        RXChar rxch2('N', false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch2);
        RXChar rxch3(' ', false, 0, carrier);
        RxBuffer::getRxBuffer()->addChar(rxch3);
    }


}
void MMVARIFrame::OnPlayBack(int a)
{
    trace(QString("%1: %2").arg("MMVARIFrame::OnPlayback(int)").arg(a));
}
void MMVARIFrame::OnClockAdjust(int &a)
{
    trace(QString("%1: %2").arg("MMVARIFrame::OnClockAdjust(int &)").arg(a));
}
void MMVARIFrame::OnError(int a)
{
    trace(QString("%1: %2").arg("MMVARIFrame::OnError(int)").arg(a));
}

void MMVARIFrame::onHandleMMVARICOMException (int code, QString source, QString desc, QString help)
{
    trace(QString("MMVARI COM/OLE error: %1 at %2: %3 (%4)").arg (QString::number(code), source, desc, help));
}

void MMVARIFrame::onMMVARISignal(const QString &/*s*/, int /*argc*/, void */*argv*/)
{
}

void MMVARIFrame::onMMVARIPropertyChanged(const QString & s)
{
    trace(QString("MMVARI onPropertyChanged: %1").arg (s));
}

void MMVARIFrame::onHandleXMMVIEWCOMException (int code, QString source, QString desc, QString help)
{
    trace(QString("XMMVIEW COM/OLE error: %1 at %2: %3 (%4)").arg (QString::number(code), source, desc, help));
}

void MMVARIFrame::onXMMVIEWSignal(const QString &/*s*/, int /*argc*/, void */*argv*/)
{
    // These are all mouse related
   // trace(QString("XMMVIEW onSignal: %1 %2").arg(s).arg(argc));
}

void MMVARIFrame::onXMMVIEWPropertyChanged(const QString & s)
{
    trace(QString("XMMVIEW onPropertyChanged: %1").arg (s));
}
