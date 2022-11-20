#include <windows.h>    // needed for shared memory

#include <QDebug>
#include <QAbstractNativeEventFilter>
#include <QDialog>
#include <QDateTime>

#include "MTrace.h"
#include "MMTTY_N1MM.h"

#include "MMTTYFrame.h"
#include "ui_MMTTYFrame.h"

#ifdef RUBBISH
enum { // APP -> MMTTY
RXM_HANDLE          // provide new window handle

RXM_REQHANDLE       // request decoder window handle
RXM_EXIT            // Terminate program
RXM_PTT             // 0=RX immediate, 1 RX when buffer empty, 2 TX, 4 clear buffer
RXM_CHAR            //send character to TX processor
    RXM_WINPOS          //Not implemented moves the display
    RXM_WIDTH           //not implemented resize display
RXM_REQPARA         //returns all switch parameters
RXM_SETBAUD         //one rate 45.454545 baud
RXM_SETMARK         //sets mark frequency - between 650 and 3000Hz
RXM_SETSPACE        // set space frequency - between 650 and 3000Hz
RXM_SETSWITCH       //setting of various switches most not applicable - squelch, Revrse shift,
RXM_SETHAM          //set default frequencies, makes AFC shift zero
RXM_SHOWSETUP       //Open setup dialog.
    RXM_SETVIEW         //not implemented show setup dialogue
    RXM_SETSQLVL        //not implemented set squelch level
RXM_SHOW            //Open setup dialog.
    RXM_SETFIG          //not implemented set FIG/LTRS shift (RX or TX?)
    RXM_SETRESO         //not implemented set filter
    RXM_SETLPF          //not implemented set LPF
    RXM_SETTXDELAY      //not implemented set PTT delay
    RXM_UPDATECOM       //not implemented shared memory update
    RXM_SUSPEND         //not implemented lets go of physical resources
    RXM_NOTCH           //not implemented set notch frequency
    RXM_PROFILE         //not implemented profile saving, loading etc
    RXM_TIMER           //not implemented PTT timer
    RXM_ENBFOCUS        //not implemented focus change enable/disable, but Inform N1MM on LTRS shift, RX
    RXM_SETDEFFREQ      //not implemented default shift and mark frequencies
    RXM_SETLENGTH       //not implemented set data bit length irrelvant
    RXM_ENBSHARED       //not implemented endable shared memory lParam=1 disable=0
    RXM_PTTFSK          //not implemented
};


// UDP Look for
// const char stringarray[8][20]= { "<timestamp>", "<band>", "<call>","<radionr>","<IsRunQSO>", "<contestname>","<mycall>", "<mode>" }; //used to compare strings against received data

// but not a lot used! 2Tone method N1MM_LANinterface
#endif



#define LONG long
#define CHAR unsigned char
//shared memory array
#pragma pack(1)  // Enable pack of structure
typedef struct {
 LONG smpFreq;    // FFT display sampling frequency (->APP)
 CHAR title[128];   // Control panel title (->MMTTY)
 CHAR comName[16];  // PTT/FSK port name(->MMTTY)
 LONG smpFFT;     // 0-11025Hz 1-8000Hz base(->MMTTY)
 LONG flagFFT;    // FFT update flag (MMTTY <-> APP)
 LONG arrayFFT[2048]; // FFT raw data (->APP)
 LONG flagXY;     // XY scope update flag (MMTTY <-> APP)
 LONG arrayX[512];  // Mark signal raw data (->APP)
 LONG arrayY[512];  // Space signal raw data (->APP)
 CHAR verMMTTY[16];  // MMTTY version number (->APP)
 CHAR RadioName[16]; // The port name for radio command (->MMTTY)
 LONG flagLostSound; // True if MMTTY has lost the sound (->APP)
 LONG flagOverflow;  // True if the input level is too high (->APP)
 LONG errorClock;   // Clock adjustment value for the sound card (ppm) (->APP)
 LONG smpDemFreq;   // Sampling frequency of the demodulator (->APP)
 LONG TxBufCount;   // The number of data that have not sent yet and remain in the TX buffer (->APP)
 CHAR ProfileName[16][64]; // Profile names
 LONG dummy[2048];  // Reserved
}COMARRAY;
#pragma pack()  // Disable pack of structure
static COMARRAY *pMap=NULL;

UINT uMSG_MMTTY = -1;

class MyNativeEventFilter: public QAbstractNativeEventFilter
{
    MMTTYFrame *m;
    HWND hWnd;
    uint msgNo;
public:
    MyNativeEventFilter(MMTTYFrame *m, uint hWnd, uint msgid):m(m), hWnd(HWND(hWnd)), msgNo(msgid)
    {}
    virtual bool nativeEventFilter(const QByteArray &/*eventType*/, void *message, long *result ) Q_DECL_OVERRIDE
    {
        MSG *msg = static_cast<MSG *>(message);
        if( msg->message == msgNo)
        {
            if ( hWnd == msg->hwnd)
            {
                m->msgEventFilter(msg, result);
                return true;
            }
        }
        return false;
    }
};


//HandleDialog::HandleDialog(QWidget *p):QDialog(p)
//{
//    // 2Tone looks for a window titled something like "DI1 RX Window 1
//    // N1MM hides it!
//    // And uses what woud be the MMVARI window?
//    // Don't include "RX Window n" if it is a main (TX) window

//    setWindowTitle( "DI2 RX Window 1");
//    t = new QWidget(this);
//}

MMTTYFrame::MMTTYFrame( bool twoTone, QTextEdit *rxChars, QLineEdit *sendEdit) :
    QFrame(nullptr),
    ui(new Ui::MMTTYFrame),
    rxChars(rxChars),
    sendEdit(sendEdit),
    twoTone(twoTone)
{
    ui->setupUi(this);

    setWindowTitle( "DI2 RX Window 1");
    t = new QWidget(this);

    // create the window to take 2Tone and MMTTY messages
    //TTYDialog = new HandleDialog(this);

    if (twoTone)
    {
        // -r remote -a stay on top -Z (?) allow multiple copies -h window handle
        runRttyEngine("C:/Ham/2Tone/2Tone.exe", QStringList());
    }
    else
    {
            // -r remote -a stay on top -Z (?) allow multiple copies -h window handle
            runRttyEngine("C:/Ham/MMTTY/MMTTY.exe", { "-r", "-Z"});
    }

}

MMTTYFrame::~MMTTYFrame()
{

    delete ui;
}

void MMTTYFrame::sendCharacters(const QString &sendData)
{
    ::PostMessage(mttyHWnd, uMSG_MMTTY, RXM_PTT, (DWORD)2);
    for(auto c:sendData)
    {
        ::PostMessage(mttyHWnd, uMSG_MMTTY, RXM_CHAR, (DWORD)c.toLatin1());
    }
    ::PostMessage(mttyHWnd, uMSG_MMTTY, RXM_PTT, (DWORD)1);
}

void MMTTYFrame::closeFrame()
{
    if (rttyProcess)
    {
        trace("About to ask engine to exit");
        ::PostMessage(mttyHWnd, uMSG_MMTTY, RXM_EXIT, 0);

        twoToneActive = false;
        mmttyActive = false;

        rttyProcess->waitForFinished(1000);

        mttyHWnd = 0;
    }
}
void MMTTYFrame::msgEventFilter(MSG *msg, long */*result*/ )
{
    if (rttyProcess)
    {
        long l = msg->lParam;
        int w = msg->wParam;

        // messages from decoder
        switch(w)
        {
        case TXM_HANDLE:
            mttyHWnd = (HWND)l;
            ::PostMessage(mttyHWnd, uMSG_MMTTY, RXM_HANDLE, (DWORD)getTempId());
            break;
        case TXM_REQHANDLE:
            ::PostMessage(mttyHWnd, uMSG_MMTTY, RXM_HANDLE, (DWORD)getTempId());
            break;
        case TXM_START:
            break;
        case TXM_CHAR:
        {
            QChar c = QChar(QLatin1Char(l & 0xff));
            rxChars->insertPlainText(QString(c));
            rxChars->ensureCursorVisible();
        }
            break;
        case TXM_PTTEVENT:
        {
            if (l == 1 && !txState)
            {
                txState = true;
                rxChars->insertPlainText(QString("\nTX\n"));
                rxChars->ensureCursorVisible();

            }
            else if (l == 0 && txState)
            {
                txState = false;
                rxChars->insertPlainText(QString("\nRX\n"));
                rxChars->ensureCursorVisible();

            }
        }
            break;
        case TXM_WIDTH:
            break;
        case TXM_BAUD:
            break;
        case TXM_MARK:
            break;
        case TXM_SPACE:
            break;
        case TXM_SWITCH:
            break;
        case TXM_VIEW:
            break;
        case TXM_LEVEL:
            break;
        case TXM_FIGEVENT:
            break;
        case TXM_RESO:
            break;
        case TXM_LPF:
            break;
        case TXM_THREAD:
            break;
        case TXM_PROFILE:
            break;
        case TXM_NOTCH:
            break;
        case TXM_DEFSHIFT:
            break;
        case TXM_RADIOFREQ:
            break;
        case TXM_SHOWSETUP:
            break;
        case TXM_SHOWPROFILE:
            break;

        default:
            break;
        }
    }
}

void MMTTYFrame::runRttyEngine(QString app, QStringList opts)
{
    rttyEngine = app;
    rttyEngineOpts = opts;

    HWND hWnd = getTempId();
    QString handleOpt = QString("-h%1").arg((UINT)hWnd, 4, 16, QChar('0'));
    rttyEngineOpts.insert(0, handleOpt);

    LPCWSTR m = L"MMTTY";
    uMSG_MMTTY = ::RegisterWindowMessageW(m); //advert & get value
    QApplication::instance()->installNativeEventFilter(new MyNativeEventFilter(this, (UINT)hWnd, uMSG_MMTTY));

    QSharedMemory hMapFile(this);
    QString k = QString("MMTTY");

    // This works... but hMapFile.create doesn't work!
    HANDLE m_hComFile = (HANDLE)::CreateFileMappingW(HANDLE(0xffffffff), NULL, PAGE_READWRITE, 0, sizeof(COMARRAY), m);
    Q_UNUSED(m_hComFile)

    hMapFile.setNativeKey(k);
    hMapFile.attach(QSharedMemory::ReadWrite);

    //get shared memory area pointer
    if (hMapFile.isAttached())
    {
        pMap = (COMARRAY *) hMapFile.data();
    }
    if (pMap != nullptr)
    {
        hMapFile.lock();
        memset(pMap,0,sizeof(COMARRAY)); //fill array with zeros
        hMapFile.unlock();
    }

    createProcess();
}
void MMTTYFrame::createProcess()
{
    rttyProcess = new QProcess(this);
    connect (rttyProcess, &QProcess::started, this, &MMTTYFrame::on_started);
    connect (rttyProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MMTTYFrame::on_finished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect (rttyProcess, &QProcess::errorOccurred, this, &MMTTYFrame::on_error);
#else
    connect (rttyProcess, &QProcess::error, this, &MMTTYFrame::on_error);
#endif

    connect (rttyProcess, &QProcess::readyReadStandardError, this, &MMTTYFrame::on_readyReadStandardError);
    connect (rttyProcess, &QProcess::readyReadStandardOutput, this, &MMTTYFrame::on_readyReadStandardOutput);

    rttyProcess->start(rttyEngine, rttyEngineOpts, QProcess::ReadWrite);
}
void MMTTYFrame::on_started()
{
    trace(rttyEngine + ":started");
}

void MMTTYFrame::on_finished(int err, QProcess::ExitStatus exitStatus)
{
    trace(rttyEngine + ":finished:" + QString::number(err) + ":" + QString::number(exitStatus));
    if (rttyProcess)
    {

        rttyProcess->closeWriteChannel();
        rttyProcess->deleteLater();
        rttyProcess = nullptr;
    }
    if (mmttyActive || twoToneActive)
    {
        createProcess();
    }
}

void MMTTYFrame::on_error(QProcess::ProcessError error)
{
    trace(rttyEngine + ":error:" + QString::number(error));
    rttyProcess->deleteLater();
    rttyProcess = nullptr;
}

void MMTTYFrame::on_readyReadStandardError()
{
    if (rttyProcess)
    {
        QString r = rttyProcess->readAllStandardError();
        trace(rttyEngine + ":stdErr:" + r);
    }
}

void MMTTYFrame::on_readyReadStandardOutput()
{
    if (rttyProcess)
    {
        QString line = rttyProcess->readLine();
        trace(rttyEngine + ":stdOut:" + line);
    }
}


