#include <QMenuBar>
#include <QMenu>
#include <QSplitter>
#include "QtUtils.h"
#include "MTrace.h"
#include "rxbuffer.h"
#include "dmmainwindow.h"
#include "enginewindow.h"
#include "engineconfigure.h"
#include "MMVARIFrame.h"

MMVARIFrame::MMVARIFrame(QFrame *cwl, EngineWindow *p,
                         QLineEdit *sendEdit,
                         int inId, int outId, QString name) :
    QObject(cwl),
    engineWindow(p),
    sendEdit(sendEdit)
{
    connect(mainWindow, &DMMainWindow::setSpeeds, this, &MMVARIFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::setSpeeds, this, &MMVARIFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::sendCharactersDown, this, &MMVARIFrame::onSendCharacters);
    connect(engineWindow, &EngineWindow::rigModeFreq, this, &MMVARIFrame::onRigModeFreq);
    connect(this, &MMVARIFrame::txChanged, engineWindow, &EngineWindow::onTxChanged);

    // NB - the OCX MUST be alongside the executable, not in the"runtime" directory (if that is different)
    // so e.g. when debugging, in
    // C:\Projects\build-maiatop-Desktop_Qt_5_15_2_MinGW_32_bit-Debug\mmvariTest\debug

    // We build in a manifest to allow Windows to find the correct OCX

    mmvari = new MMVARILib::MMVARI();
    mmvari->setControl("{9C0D49DD-5C05-456D-916B-98C4CF63172F}");

    mmview = new MMVARILib::XMMVView(cwl);
    mmview->setControl("{702CBF07-C159-44F2-B8A6-DF8EA1001E08}");

    mmview2 = new MMVARILib::XMMVView(cwl);
    mmview2->setControl("{702CBF07-C159-44F2-B8A6-DF8EA1001E08}");

    mmlevel = new MMVARILib::XMMVLvl(cwl);
    mmlevel->setControl("{438EF93A-939D-4B6B-93A7-DF09049B8514}");


    //====================================================================
    // N1MM also has BPF, ATC, FFT, Multi-Channel RX menus
    // Do we need to have anything here?

    QMenuBar *mmbar = new QMenuBar(cwl);
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

    // until we actually implement these controls, don't show them
    //mmvariVb->addWidget(mmbar);

    //====================================================================
    // we need a row of buttons here...

    // to match N1MM+ we need buttons

    // TX, RX, AFC, NET, ALIGN, mode combo, speed combo, TX Carrier, RX Carrier, s/n indicator

    QHBoxLayout *mmvariButtons = new QHBoxLayout();

    txLabel = new QLabel();
    txLabel->setText("RX");
    txLabel->setToolTip(tr("RX/TX state"));
    mmvariButtons->addWidget(txLabel);
    QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    mmvariButtons->addItem(horizontalSpacer);

    afcButton = new QPushButton();
    afcButton->setCheckable(true);
    afcButton->setText("AFC");
    afcButton->setToolTip(tr("Move the cursor to keep it centered on a signal if the frequency changes slightly"));
    mmvariButtons->addWidget(afcButton);
    connect(afcButton, &QPushButton::toggled, this, &MMVARIFrame::afcButtonToggled);

    netButton = new QPushButton();
    netButton->setCheckable(true);
    netButton->setText("NET");
    netButton->setToolTip(tr("When NET is on the TX frequency follows the RX frequency "));
    mmvariButtons->addWidget(netButton);
    connect(netButton, &QPushButton::toggled, this, &MMVARIFrame::netButtonToggled);

    alignButton = new QPushButton();
    alignButton->setText("Align");
    alignButton->setToolTip(tr("Align the transmit frequency to the receive frequency"));
    mmvariButtons->addWidget(alignButton);
    connect(alignButton, &QPushButton::clicked, this, &MMVARIFrame::alignButtonClicked);


    modeCombo = new QComboBox();
    mmvariButtons->addWidget(modeCombo);

    speedCombo = new QComboBox(cwl);
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

    mmvariVb = new QVBoxLayout();

    mmvariVb->addLayout(mmvariButtons);

    modeCombo->setCurrentIndex(0);
    onModeComboChanged(modeCombo->currentText());

    // Set the PTT options

    QString PTTPort = EngineConfigure::getEnginePTT(name);
    if (!PTTPort.isEmpty())
    {
        int PTTLine = EngineConfigure::getEnginePTTL(name); // 0 is RTS, 1 is DTR (2 is TXD)

        mmvari->setStrPTTPort(PTTPort);
        mmvari->setBPTTLock(false);
        mmvari->setBPTTLines(MMVARILib::pttlineDTR, PTTLine == 1);
        mmvari->setBPTTLines(MMVARILib::pttlineRTS, PTTLine == 0);
    }
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

    trace(QString("MMVARI devs selection %1").arg(devs));

    mmvari->setStrSoundID(devs);

    mmvari->setStrMode(0, modeCombo->currentText());
    mmvari->setWCharset(0, mmvari->wDefaultCharset());

    mmview->setWType(MMVARILib::MMVX_VIEWTYPE::viewtypeFFT);
    mmview->setWWidthFreq(3000);    // frequency span in spectrum window

    //*********************
    mmview->setBLSB(true);

    mmvari->setBActive(true);

    cwl->setLayout(mmvariVb);

    bool bAFC = mmvari->bAFC(0);
    bool bNET = mmvari->bNET();

    afcButton->setChecked(bAFC);
    netButton->setChecked(bNET);
}

MMVARIFrame::~MMVARIFrame()
{

}
void MMVARIFrame::onSetSpeeds(QString b, QString r)
{
    bpskSpeed = b;
    rttySpeed = r;

    if (modeCombo->currentText().contains("bpsk"))
    {
        if (b.contains("31"))
        {
            speedCombo->setCurrentText("31.25");
        }
        else
        {
            speedCombo->setCurrentText("62.5");
        }
    }
    else if (modeCombo->currentText().contains("rtty"))
    {
        if (r.contains("45"))
        {
            speedCombo->setCurrentText("45.45");
        }
        else
        {
            speedCombo->setCurrentText("75");
        }
    }
}

void MMVARIFrame::onSendCharacters(QString data, int markfreq)
{
    trace(QString("onSendCharacters %1 mark %2").arg(data).arg(markfreq));
    sendCharacters(data, markfreq);
}

void MMVARIFrame::onRigModeFreq(QString mode, Frequency f)
{
    if (mode == RY)
    {
        Frequency rttyOffset = engineWindow->getRttyOffset();
        f = f + Frequency(rttyOffset);
    }
    else if (mode == PSK)
    {
        Frequency pskOffset = engineWindow->getPSKOffset();
        f = f + Frequency(pskOffset);
    }
    mmview->setDwFreqHz(f.toInt64());  // tranciever frequency
    sendMode(mode);

}
void MMVARIFrame::sendCharacters(const QString &sendData, int mf)
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
        mmvari->setWBufferCount(0);// and clear out the TX buffer
    }
    else
    {
        txLabel->setText("TX");

        RXChar rxchn('\n', 0, markfreq);
        engineWindow->rxBuff.addChar(rxchn);
        // RXChar rxch('T', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch);
        // RXChar rxch2('X', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch2);
        // RXChar rxch3(' ', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch3);

        if (modeCombo->currentText().contains("rtty"))
        {
            if (mf > 0)
            {
                mmvari->setWTxCarrier(mf + RttyMSGap/2);
                mmvari->setWRxCarrier(0, mf + RttyMSGap/2);
            }
        }
        mmvari->setBAddStartCR(true);
        mmvari->setBAddStopCR(true);
        mmvari->SendText(sendData);
        mmvari->setBTX(true);
        mmvari->setBReqRX(true);    // return to RX when buffer empty
    }
}

void MMVARIFrame::sendMode(QString m)
{
    if (m == PSK)
    {
        mmview->setBLSB(false);
        modeCombo->setCurrentText("bpsk");
    }
    if (m == RY)
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
    }
    else
    {
        mmview->setBLSB(false);

        speedCombo->addItem("31.25");
        speedCombo->addItem("62.5");
    }
    onSetSpeeds(bpskSpeed, rttySpeed);
}
void MMVARIFrame::onSpeedComboChanged(const QString &s)
{
    mmvari->setDblSpeed(0, s.toDouble());
}
void MMVARIFrame::afcButtonToggled(bool /*checked*/)
{
    bool checked = afcButton->isChecked();
    trace(QString("set net to %1").arg(checked));
    mmvari->setBAFC(0, checked);
}
void MMVARIFrame::netButtonToggled(bool /*checked*/)
{
    bool checked = netButton->isChecked();
    trace(QString("set net to %1").arg(checked));
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
    int mf = txc;
    if (modeCombo->currentText().contains("rtty"))
    {
        mf -= 170/2;
    }
    trace(QString("OnTxCarrier c %1 m %2").arg(txc).arg(mf));
    txCarrier->setText(QString("Tx %1").arg(txc));
}

void MMVARIFrame::OnRxCarrier(int /*rxChannel*/, int rxc)
{
    markfreq = rxc;
    trace(QString("OnRxCarrier c %1 m %2").arg(rxc).arg(markfreq));
    rxCarrier->setText(QString("Rx %1").arg(rxc));
    if (modeCombo->currentText().contains("rtty"))
    {
        markfreq -= 170/2;
    }
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

    trace(QString("RX chars %1 mark %2").arg(strChar).arg(markfreq));
    for (const auto &c:QASCONST(strChar))
    {
        RXChar rxch(c, 0, markfreq);
        engineWindow->rxBuff.addChar(rxch);
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
void MMVARIFrame::OnNET(int a)
{
    trace(QString("%1: %2").arg("MMVARIFrame::OnNet(int)").arg(a));
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
        txLabel->setText("RX");
//        txButton->setChecked(false);
//        rxButton->setChecked(true);

        // RXChar rxchn('\n', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxchn);
        // RXChar rxch('R', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch);
        // RXChar rxch2('X', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch2);
        // RXChar rxch3(' ', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch3);

        emit txChanged(false);
    }
    else if (a == 1)
    {
        // should have happened earlier
//        txLabel->setText("TX");

//        RXChar rxchn('\n', 0, markfreq);
//        engineWindow->rxBuff.addChar(rxchn);
//        RXChar rxch('T', 0, markfreq);
//        engineWindow->rxBuff.addChar(rxch);
//        RXChar rxch2('X', 0, markfreq);
//        engineWindow->rxBuff.addChar(rxch2);
//        RXChar rxch3(' ', 0, markfreq);
//        engineWindow->rxBuff.addChar(rxch3);

        emit txChanged(true);
    }
    else if (a == 2 || a == 3)
    {
        txLabel->setText("Wait");
//        txButton->setChecked(true);
//        rxButton->setChecked(false);
        //RXChar rxchn('\n', 0, markfreq);
        //engineWindow->rxBuff.addChar(rxchn);
        // RXChar rxch('W', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch);
        // RXChar rxch2('T',  0, markfreq);
        // engineWindow->rxBuff.addChar(rxch2);
        // RXChar rxch3(' ', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch3);
        emit txChanged(true);
    }
    else if (a == 4)
    {
        txLabel->setText("Tone");
//        txButton->setChecked(true);
//        rxButton->setChecked(false);
        RXChar rxchn('\n', 0, markfreq);
        engineWindow->rxBuff.addChar(rxchn);
        // RXChar rxch('T', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch);
        // RXChar rxch2('N', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch2);
        // RXChar rxch3(' ', 0, markfreq);
        // engineWindow->rxBuff.addChar(rxch3);
        emit txChanged(true);
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
