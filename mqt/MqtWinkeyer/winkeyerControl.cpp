/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
//                      Based on example code from K1EL.
//
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#include <QDebug>
#include "winkeyerControl.h"
#include "TxThread.h"
#include "RxThread.h"
#include "winKeyerCommon.h"

WinkeyerControl::WinkeyerControl(QObject *parent)
    : QObject(parent),
    serialPort(new QSerialPort(this)),
    txThread(new TxThread(this)),
    rxThread(new RxThread(serialPort))
{
    connect(rxThread, &RxThread::dataReceived, this, &WinkeyerControl::handleDataReceived);
    connect(txThread, &TxThread::writeData, this, [=](const QByteArray &data) {serialPort->write(data);});

    // current WinKeyer Settings
    currentWinkeyerStatePtr = QSharedPointer<WinkeyerState>::create();
    currentWinkeyStateStoragePtr = QSharedPointer<WinkeyerStateStorage>::create();
    currentWinkeyStateStoragePtr->setWkstate(currentWinkeyerStatePtr);

    // new WinKeyer Settings from setupdialog
    newWinkeyerStatePtr = QSharedPointer<WinkeyerState>::create();
    newWinkeyStateStoragePtr = QSharedPointer<WinkeyerStateStorage>::create();
    newWinkeyStateStoragePtr->setWkstate(newWinkeyerStatePtr);

    QString fileName = WINKEYER_PATH_LOGGER() + WINKEYER_CONFIG_FILENAME;
    QSettings  winkeyerConfig(fileName, QSettings::IniFormat);
    currentWinkeyStateStoragePtr->loadWinkeyerStateStorageFromFile(winkeyerConfig);
    newWinkeyStateStoragePtr->loadWinkeyerStateStorageFromFile(winkeyerConfig);     // make sure they are both the same at start

    serialTimeoutTimer = new QTimer(this);
    serialTimeoutTimer->setSingleShot(true);


}


WinkeyerControl::~WinkeyerControl()
{
    stop();
}





QSharedPointer<WinkeyerStateStorage> WinkeyerControl::getCurrentWinkeyStateStoragePtr()
{
    return currentWinkeyStateStoragePtr;
}

void WinkeyerControl::setNewWinkeyStateStoragePtr(QSharedPointer<WinkeyerStateStorage> newPtr) {
    newWinkeyStateStoragePtr = newPtr;
}

QSharedPointer<WinkeyerStateStorage> WinkeyerControl::getNewWinkeyStateStoragePtr()
{
   return newWinkeyStateStoragePtr;
}


int WinkeyerControl::initComport(QString comport, int baudrate)
{

    serialPort->setPortName(comport);
    serialPort->setBaudRate(baudrate);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::SoftwareControl);
    if (serialPort->open(QIODevice::ReadWrite))
    {
        return SERIAL_OK;
    }

    return SERIALPORT_FAILED_TO_OPEN;
        //errMsg = serialPort->errorString();


}


QString WinkeyerControl::getSerialPortErrorMsg()
{
    return serialPort->errorString();
}





void WinkeyerControl::start()
{

    rxThread->start();
    txThread->start();

}

void WinkeyerControl::stop()
{
    if (serialPort->isOpen())
    {
        serialPort->close();
    }
    txThread->quit();
    txThread->wait();
    rxThread->quit();
    rxThread->wait();
}

bool WinkeyerControl::isSerialPortOpen()
{
    return serialPort->isOpen();
}

void WinkeyerControl::serialPortClose()
{
    serialPort->close();
}

bool WinkeyerControl::getIsWkOpen()
{
    return wkIsOpen;
}

void WinkeyerControl::setIsWKOpen(bool open)
{
    wkIsOpen = open;
}


void WinkeyerControl::handleDataReceived(const QByteArray &data)
{

    for (quint8 value : data)
    {
        qDebug() << "received data = " << value;
        if (activeSerialCmd != ADMIN_NONE)
        {
            if (activeSerialCmd == ADMIN_ECHO)
            {
                if (value == '\x55')
                {
                    // echo byte received send open cmd
                    stopSerialTimeout();
                    sendOpenWinKeyerCmd();
                }
            }
            else if (activeSerialCmd == ADMIN_OPEN)
            {
                stopSerialTimeout();
                wkVersion = value;
                if (ignoreExtraDataOnOpening)
                {
                    completeOpenCmd();
                }
                else
                {
                    getWinKeyerMinor();
                }
            }
            else if (activeSerialCmd == ADMIN_GETMINOR)
            {
                stopSerialTimeout();
                wkMinor = value;
                getWinKeyerType();
            }
            else if (activeSerialCmd == ADMIN_GETTYPE)
            {
                stopSerialTimeout();
                chipType = value;
                completeOpenCmd();
            }
        }
        else
        {

            if ((value & 0xC0) == 0xC0)
            {
                qDebug() << "set wkStatus1 = " << (value & 0x3f);
                setWkStatus1(value & 0x3f);

                if (getWkStatus1() & BREAKIN)
                {
                    setBreakinClear(true);
                }

                statusChanged(getWkStatus1());
            }
            else if (value & 0x80)
            {
                setWkSpeedPot((value & 0x7f) + getCurrentWinkeyStateStoragePtr()->getWkState()->getMinwpm());
                speedPotChanged(getWkSpeedPot());
            }
            else
            {
                SetWEchoBack(value);
                // place char into a echo buffer???????????????
                echoBackReceived(value);
            }

        }
    }

}




void WinkeyerControl::enqueueData(const QByteArray &data)
{
    QMutexLocker locker(&mutex);
    txQueue.enqueue(data);
    txCondition.wakeOne();
    qDebug() << "Data added to TxQueue and thread notified";
}

void WinkeyerControl::wakeUpTxThread()
{
    QMutexLocker locker(&mutex);
    txCondition.wakeOne();
    qDebug() << "Setup changed and thread notified";
}

void WinkeyerControl::closeWinKeyer()
{

    winKeyerOpenFlag = false;
}


void WinkeyerControl::openWinKeyer()
{
    // This kicks off a sequence of commands to open winkeyer
    testWinKeyerConnected();
}


void WinkeyerControl::testWinKeyerConnected()
{
    QByteArray buf;

    buf.append(ADMIN_CMD);
    buf.append(ADMIN_ECHO);
    buf.append('\x55');

    startSerialTimeout(ADMIN_ECHO);
    enqueueData(buf);
}

void WinkeyerControl::sendOpenWinKeyerCmd()
{
    QByteArray buf;

    buf.append(ADMIN_CMD);
    buf.append(ADMIN_OPEN);
    startSerialTimeout(ADMIN_OPEN);
    enqueueData(buf);
}

void WinkeyerControl::getWinKeyerMinor()
{
    QByteArray buf;

    buf.append(ADMIN_CMD);
    buf.append(ADMIN_GETMINOR);
    startSerialTimeout(ADMIN_GETMINOR);
    enqueueData(buf);
}

void WinkeyerControl::getWinKeyerType()
{
    QByteArray buf;

    buf.append(ADMIN_CMD);
    buf.append(ADMIN_GETTYPE);
    startSerialTimeout(ADMIN_GETTYPE);
    enqueueData(buf);
}

void WinkeyerControl::startSerialTimeout(quint8 currentCmd)
{
    activeSerialCmd = currentCmd;
    serialTimeoutTimer->start(serialTimeoutInterval);

}

void WinkeyerControl::statusChanged(quint8 status)
{
    // Process status

    quint8 g_wkStatus = status;

    if (g_wkStatus & KPBSTAT)
    {
        if (g_wkStatus & KPB1)
        {

        }
        else if (g_wkStatus & KPB2)
        {

        }
        else if (g_wkStatus & KPB3)
        {

        }
        else if (g_wkStatus & KPB4)
        {

        }
        else
        {
            //SetDlgItemText(g_hMainDlg, IDC_MSG1, "M1");
            //SetDlgItemText(g_hMainDlg, IDC_MSG2, "M2");
            //SetDlgItemText(g_hMainDlg, IDC_MSG3, "M3");
            //SetDlgItemText(g_hMainDlg, IDC_MSG4, "M4");
        }
    }

    if (g_wkStatus & XOFF)
    {
        emit wk_XoffStatus("Xoff");

    }
    else
    {
        emit wk_XoffStatus("    ");
    }


    if (g_wkStatus & BREAKIN)
    {
        emit wk_BreakInStatus("BrkIn");
    }
    else
    {
        emit wk_BreakInStatus("     ");
    }

    if (g_wkStatus & KBUSY)
    {
        emit wk_KBusyStatus("Busy");
    }
    else
    {
        emit wk_KBusyStatus("    ");
    }

    if (g_wkStatus & KWAIT)
    {
        emit wk_KWaitStatus("Wait");
    }
    else
    {
        emit wk_KWaitStatus("    ");
    }
}

void WinkeyerControl::speedPotChanged(quint8 speedPot)
{
    quint8 wkSpeedPot = speedPot;
    if (getPotLock())
    {
        // need to update wpm on main dialogue here!!!!!!!!!!!!!!!!!!!!!!!!
        wkSetWpmSpeed(wkSpeedPot);
    }

}

void WinkeyerControl::echoBackReceived(quint8 echoBack)
{
    // Process echo back

}



void WinkeyerControl::completeOpenCmd()
{
    // open sequence complete
    activeSerialCmd = ADMIN_NONE;
    winKeyerOpenFlag = true;
    emit winKeyerOpenStatus(true);

}

void WinkeyerControl::stopSerialTimeout()
{
    serialTimeoutTimer->stop();
    activeSerialCmd = ADMIN_NONE;
}

QString WinkeyerControl::getAdminCmdText(quint8 cmdNum)
{
    if (cmdNum < adminCmdNames.length())
    {
        return adminCmdNames[cmdNum];
    }

    return "invalid cmdNum";

}

void WinkeyerControl::handleSerialTimeout()
{
    QString cmdText = getAdminCmdText(activeSerialCmd);
    activeSerialCmd = ADMIN_NONE;
    qDebug() << "Serial Timeout - Cmd = " << cmdText;
}

void WinkeyerControl::setWkStatus1(quint8 value)
{
    wkStatus1 = value;
}
quint8 WinkeyerControl::getWkStatus1()
{
    return wkStatus1;
}
void WinkeyerControl::SetWEchoBack(quint8 value)
{
    wEchoBack = value;
}
quint8 WinkeyerControl::getWEchoBack()
{
    return wEchoBack;
}

void WinkeyerControl::setWkSpeedPot(quint8 value)
{
    wkSpeedPot = value;
}
quint8 WinkeyerControl::getWkSpeedPot()
{
    return wkSpeedPot;
}
void WinkeyerControl::setDoGetPot(bool state)
{
    doGetPot = state;
}
bool WinkeyerControl::getDoGetPot()
{
    return doGetPot;
}
void WinkeyerControl::setDoBlock(bool state)
{
    doBlock = state;
}
bool WinkeyerControl::getDoBlock()
{
    return doBlock;
}
void WinkeyerControl::setDumpPot(bool state)
{
    dumpPot = state;
}
bool WinkeyerControl::getDumpPot()
{
    return dumpPot;
}
void WinkeyerControl::setPotLock(bool state)
{
    potLock = state;
}
bool WinkeyerControl::getPotLock()
{
    return potLock;
}
void WinkeyerControl::setDumpPdl(bool state)
{
    dumpPdl = state;
}
bool WinkeyerControl::getDumpPdl()
{
    return dumpPdl;
}
void WinkeyerControl::setDumpDebug(bool state)
{
    dumpDebug = state;
}
bool WinkeyerControl::getDumpDebug()
{
    return dumpDebug;
}
void WinkeyerControl::setDumpState(bool state)
{
    dumpState = state;
}
bool WinkeyerControl::getDumpState()
{
    return dumpState;
}
void WinkeyerControl::setBreakinClear(bool state)
{
    breakinClear = state;
}
bool WinkeyerControl::getBreakinClear()
{
    return breakinClear;
}
void WinkeyerControl::setNewVolume(bool state)
{
    newVolume = state;
}
bool WinkeyerControl::getNewVolume()
{
    return newVolume;
}
void WinkeyerControl::setNewX2mode(bool state)
{
    newX2mode = state;
}
bool WinkeyerControl::getNewX2mode()
{
    return newX2mode;
}
void WinkeyerControl::setNewRTTY(bool state)
{
    newRTTY = state;
}
bool WinkeyerControl::getNewRTTY()
{
    return newRTTY;
}
void WinkeyerControl::setNewClear(bool state)
{
    newClear = state;
}
bool WinkeyerControl::getNewClear()
{
    return newClear;
}
void WinkeyerControl::setCurPause(bool state)
{
    curPause = state;
}
bool WinkeyerControl::getCurPause()
{
    return curPause;
}
void WinkeyerControl::setNewPause(bool state)
{
    newPause = state;
}
bool WinkeyerControl::getNewPause()
{
    return newPause;
}
void WinkeyerControl::setcurTune(bool state)
{
    curTune = state;
}
bool WinkeyerControl::getcurTune()
{
    return curTune;
}
void WinkeyerControl::setNewTune(bool state)
{
    newTune = state;
}
bool WinkeyerControl::getNewTune()
{
    return newTune;
}
void WinkeyerControl::setCurDirect(quint8 direct_)
{
    curDirect = direct_;
}
quint8 WinkeyerControl::getCurDirect()
{
    return curDirect;
}

void WinkeyerControl::setNewDirect(quint8 direct_)
{
    newDirect = direct_;
}
quint8 WinkeyerControl::getNewDirect()
{
    return newDirect;
}
void WinkeyerControl::setHscwValue(quint8 value)
{
    hscwValue = value;
}
quint8 WinkeyerControl::getHscwValue()
{
    return hscwValue;
}
void WinkeyerControl::setX2modeValue(quint8 value)
{
    x2modeValue = value;
}
quint8 WinkeyerControl::getX2modeValue()
{
    return x2modeValue;
}
void WinkeyerControl::setVolumeValue(quint8 value)
{
    volumeValue = value;
}
quint8 WinkeyerControl::getVolumeValue()
{
    return volumeValue;
}
void WinkeyerControl::setRttyValue1(quint8 value)
{
    rttyValue1 = value;
}
quint8 WinkeyerControl::getRttyValue1()
{
    return rttyValue1;
}
void WinkeyerControl::setRttyValue2(quint8 value)
{
    rttyValue2 = value;
}
quint8 WinkeyerControl::getRttyValue2()
{
    return rttyValue2;
}
void WinkeyerControl::setDoSoftReset(bool state)
{
    doSoftReset = state;
}
bool WinkeyerControl::getDoSoftReset()
{
    return doSoftReset;
}
void WinkeyerControl::setDoBackSpace(bool state)
{
    doBackSpace = state;
}
bool WinkeyerControl::getDoBackSpace()
{
    return doBackSpace;
}
void WinkeyerControl::setCloseTXFlag(bool state)
{
    closeTx = state;
}
bool WinkeyerControl::getCloseTxFlag()
{
    return closeTx;
}
void WinkeyerControl::setSetHscwFlag(bool state)
{
    setHscw = state;
}
bool WinkeyerControl::getSetHscwFlag()
{
    return setHscw;
}
void WinkeyerControl::setWK1Flag(bool state)
{
    setWK1 = state;
}
bool WinkeyerControl::getWK1Flag()
{
    return setWK1;
}
void WinkeyerControl::setWK2Flag(bool state)
{
    setWK2 = state;
}
bool WinkeyerControl::getWK2Flag()
{
    return setWK2;
}
void WinkeyerControl::setWK3Flag(bool state)
{
    setWK3 = state;
}
bool WinkeyerControl::getWK3Flag()
{
    return setWK3;
}

// immediate commands


// set sidetone frequency


int WinkeyerControl::wkSendDefaults(QSharedPointer<WinkeyerStateStorage> state)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setModereg(state->getWkState()->getModereg());
        currentWinkeyStateStoragePtr->getWkState()->setModereg(state->getWkState()->getModereg());

        newWinkeyStateStoragePtr->getWkState()->setSpeed(state->getWkState()->getSpeed());
        currentWinkeyStateStoragePtr->getWkState()->setSpeed(state->getWkState()->getSpeed());

        newWinkeyStateStoragePtr->getWkState()->setStconst(state->getWkState()->getStconst());
        currentWinkeyStateStoragePtr->getWkState()->setStconst(state->getWkState()->getStconst());

        newWinkeyStateStoragePtr->getWkState()->setWeight(state->getWkState()->getWeight());
        currentWinkeyStateStoragePtr->getWkState()->setWeight(state->getWkState()->getWeight());

        newWinkeyStateStoragePtr->getWkState()->setLeadin(state->getWkState()->getLeadin());
        currentWinkeyStateStoragePtr->getWkState()->setLeadin(state->getWkState()->getLeadin());

        newWinkeyStateStoragePtr->getWkState()->setTail(state->getWkState()->getTail());
        currentWinkeyStateStoragePtr->getWkState()->setTail(state->getWkState()->getTail());

        newWinkeyStateStoragePtr->getWkState()->setMinwpm(state->getWkState()->getMinwpm());
        currentWinkeyStateStoragePtr->getWkState()->setMinwpm(state->getWkState()->getMinwpm());

        newWinkeyStateStoragePtr->getWkState()->setWpmrange(state->getWkState()->getWpmrange());
        currentWinkeyStateStoragePtr->getWkState()->setWpmrange(state->getWkState()->getWpmrange());

        newWinkeyStateStoragePtr->getWkState()->setXtnd(state->getWkState()->getXtnd());
        currentWinkeyStateStoragePtr->getWkState()->setXtnd(state->getWkState()->getXtnd());

        newWinkeyStateStoragePtr->getWkState()->setKcomp(state->getWkState()->getKcomp());
        currentWinkeyStateStoragePtr->getWkState()->setKcomp(state->getWkState()->getKcomp());

        newWinkeyStateStoragePtr->getWkState()->setFarns(state->getWkState()->getFarns());
        currentWinkeyStateStoragePtr->getWkState()->setFarns(state->getWkState()->getFarns());

        newWinkeyStateStoragePtr->getWkState()->setSampadj(state->getWkState()->getSampadj());
        currentWinkeyStateStoragePtr->getWkState()->setSampadj(state->getWkState()->getSampadj());

        newWinkeyStateStoragePtr->getWkState()->setDitdahratio(state->getWkState()->getDitdahratio());
        currentWinkeyStateStoragePtr->getWkState()->setDitdahratio(state->getWkState()->getDitdahratio());

        newWinkeyStateStoragePtr->getWkState()->setPincfg(state->getWkState()->getPincfg());
        currentWinkeyStateStoragePtr->getWkState()->setPincfg(state->getWkState()->getPincfg());

        newWinkeyStateStoragePtr->getWkState()->setX1mode(state->getWkState()->getX1mode());
        currentWinkeyStateStoragePtr->getWkState()->setX1mode(state->getWkState()->getX1mode());

        setDoBlock(true);       // kick off sending data to winkeyer

        return WK_SUCCESS;
    }
    else
    {
        return WK_NOT_OPEN;
    }
}

// set sidetone frequency

int WinkeyerControl::wkSetSidetoneFreq(quint8 value)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setSpeed(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// set Morse sending speed
int WinkeyerControl::wkSetWpmSpeed(quint8 value)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setSpeed(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// set morse weighting

int WinkeyerControl::wkSetWeight(quint8 value)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setWeight(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// set ptt lead, tail

int WinkeyerControl::wkSetPttLeadTail(quint8 lead, quint8 tail)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setLeadin(lead);
        newWinkeyStateStoragePtr->getWkState()->setTail(tail);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// setp WK's speed pot characteristics

int WinkeyerControl::wkSetupSpeedPot(quint8 minwpm, quint8 wpmrange/*, quint8 potrange*/)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setMinwpm(minwpm);
        newWinkeyStateStoragePtr->getWkState()->setWpmrange(wpmrange);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// pause morse sending

int WinkeyerControl::wkPauseCw(bool state)
{
    if (wkIsOpen)
    {
        setNewPause(state);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

int WinkeyerControl::wkBackspace()
{
    if (wkIsOpen)
    {
        setDoBackSpace(true);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// set output config

int WinkeyerControl::wkPincfg(quint8 value)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setPincfg(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// clear wk's input buffer

int WinkeyerControl::wkPincfg()
{
    if (wkIsOpen)
    {
        setNewClear(true);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// set new X2Mode register

int WinkeyerControl::wkSetX2Mode(quint8 x2mode)
{
    if (wkIsOpen)
    {
        setNewX2mode(true);
        setX2modeValue(x2mode);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// set new Sidetone Volume

int WinkeyerControl::wkSetStVolume(quint8 volume)
{
    if (wkIsOpen)
    {
        setNewVolume(true);
        setVolumeValue(volume);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}


// set New RTTY mode

int WinkeyerControl::wkSetRTTY(quint8 WKrtty, quint8 RYmode)
{
    if (wkIsOpen)
    {
        setNewRTTY(true);
        setRttyValue1(WKrtty);
        setRttyValue2(RYmode);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// wk_key state
// true = key down
// false = key up

int WinkeyerControl::wkKey(bool state)
{
    if (wkIsOpen)
    {
        setNewTune(state);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// set farnsworth speed in wpm

int WinkeyerControl::wkSetFarns(quint8 speed)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setFarns(speed);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}


// set wk mode register

int WinkeyerControl::wkSetMode(quint8 value)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setModereg(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// set keying compensation

int WinkeyerControl::wkSetKComp(quint8 value)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setKcomp(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}


// set key sampling action

int WinkeyerControl::wkKeySample(quint8 value)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setSampadj(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// wk_directkey(value)  firmware paddle control


int WinkeyerControl::wkDirectKey(quint8 value)
{
    if (wkIsOpen)
    {
        setNewDirect(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// Get WK's status

int WinkeyerControl::wkGetStatus()
{
    if (wkIsOpen)
    {

        return getWkStatus1();
    }

    return WK_NOT_OPEN;
}

//get WK's speed pot value

int WinkeyerControl::wkGetSpeedPot()
{
    if (wkIsOpen)
    {
        setDoGetPot(true);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// get WK's echo

int WinkeyerControl::wkGetEcho()
{
    //quint8 = echo;

    if (wkIsOpen)
    {

    }

    return WK_NOT_OPEN;
}



// set dit/dah ratio

int WinkeyerControl::wkSetDitDahRation(quint8 value)
{
    if (wkIsOpen)
    {
        newWinkeyStateStoragePtr->getWkState()->setDitdahratio(value);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}


// get WK's local buffer XON/OFF status

int WinkeyerControl::wkGetXoffStatus()
{
    if (wkIsOpen)
    {

    }

    return WK_NOT_OPEN;
}

// send a buffered character to WK

int WinkeyerControl::wkSendBufferedChar(quint8 ch)
{
    if (wkIsOpen)
    {
        QByteArray c;
        c.append(ch);
        enqueueData(c);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
    return WK_SUCCESS;
}

// set/clear Wk's PTT Output

int WinkeyerControl::wkSetBuffPtt(bool state)
{
    if (wkIsOpen)
    {
        wkSendBufferedChar(BUFPTT_CMD);
        wkSendBufferedChar(state ? 1 :0);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}


// set/clear WK's Key Output

int WinkeyerControl::wkSetBufKey(bool state)
{
    if (wkIsOpen)
    {
        wkSendBufferedChar(BUFKEY_CMD);
        wkSendBufferedChar(state ? 1: 0);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// Start a WK buffered wait

int WinkeyerControl::wkStartBufferedWait(quint8 sec)
{
    if (wkIsOpen)
    {
        wkSendBufferedChar(WAIT_CMD);
        wkSendBufferedChar(sec);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// merge two characters into a prosign

int WinkeyerControl::wkMergeChar(quint8 c1, quint8 c2)
{
    if (wkIsOpen)
    {
        wkSendBufferedChar(MERGE_CMD);
        wkSendBufferedChar(c1);
        wkSendBufferedChar(c2);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// Insert a buffered speed change

int WinkeyerControl::wkBufferedSpeedChange(quint8 wpm)
{
    if (wkIsOpen)
    {
        wkSendBufferedChar(BUFSPEED_CMD);
        wkSendBufferedChar(wpm);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}

// Insert a buffered HSCW speed Change

int WinkeyerControl::wkBufferedHscw(quint8 wpm)
{
    if (wkIsOpen)
    {
        wkSendBufferedChar(BUFHSCW_CMD);
        wkSendBufferedChar(wpm);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}


// Cancel a buffered speed change

int WinkeyerControl::wkCancelBufferedWpm()
{
    if (wkIsOpen)
    {
        wkSendBufferedChar(CANCELSPD_CMD);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}


// process a buffered null (NOP)

int WinkeyerControl::wkBufferedNull()
{
    if (wkIsOpen)
    {
        wkSendBufferedChar(NULLBUF_CMD);
        return WK_SUCCESS;
    }

    return WK_NOT_OPEN;
}
