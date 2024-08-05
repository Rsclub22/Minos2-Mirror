/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WINKEYERCOMMON_H
#define WINKEYERCOMMON_H

/*****************************************************************************
 *
 *  WinKeyer Constants Courtesey K1EL
 *
 * ***************************************************************************/

#include <QtGlobal>
#include <QStringList>
#include <QSharedPointer>
#include <QSettings>
#include <QDebug>

/*****************************************
*       Winkey status byte
* ****************************************/

const quint8 LOCALXOFF = 0x80;      // XOFF bit set by DLL
const quint8 KWAIT =     0x10;      // Timed wait when = 1
const quint8 KTUNE =     0x08;      // Keydown when = 1
const quint8 KBUSY =     0x04;      // Keyer busy when = 1
const quint8 BREAKIN =   0x02;      // Break in when = 1
const quint8 XOFF =      0x01;      // XOFF when = 1

/* WK2 PB Status */

const quint8 KPB4 =      0x10;      // PB 4 on flag
const quint8 KPBSTAT =   0x08;      // Pushbutton status when = 1
const quint8 KPB3 =      0x04;      // PB 3 on flag
const quint8 KPB2 =      0x02;      // PB 2 on flag
const quint8 KPB1 =      0x01;      // PB 1 on flag


/*****************************************
*       PINCFG byte
*****************************************/

const quint8 DITPRI =    0x80;       // Dit insertion mode
const quint8 DAHPRI =    0x40;       // Dah insertion mode
const quint8 HANG1 =     0x20;       // Paddle Hang MSbit
const quint8 HANG0 =     0x10;       // Paddle Hang LSbit
const quint8 KEYPORT1 =  0x08;       // Use Keyport 1, no PTT
const quint8 KEYPORT0 =  0x04;       // Use Keyport 0, no PTT
const quint8 USESTONE =  0x02;       // Pin 5 = Sidetone
const quint8 USEPTT =    0x01;       // Pin 5 = PTT

/*****************************************
*       Winkey mode byte
*****************************************/

const quint8 NONSTICK =  0x80;       // Disable paddle watchdog when = 1
const quint8 PDL_ECHO =  0x40;       // Paddle echo enabled when = 1
const quint8 NIAMBIC =   0x20;       // Iambic mode when 0
const quint8 KEYERMODE = 0x10;       // if NIAMBIC=0 then 0=B/1=A \
                                     // if NIAMBIC=1 then 0=Ultimatic/1=Bug mode
const quint8 PDL_SWAP =  0x08;       // Swap paddle left/right
const quint8 SER_ECHO =  0x04;       // Serial echo enabled when = 1
const quint8 ASPACE =    0x02;       // Autospacing enabled when = 1
const quint8 CT_SPACE =  0x01;       // Contest spacing enabled when = 1

/*****************************************
*       Winkey xmode1 byte
*****************************************/

const quint8 USER =      0x80;       // User 0/1 select
const quint8 MSGBANKO =  0x40;       // Message bank 0/1 select
const quint8 TUNE50 =    0x20;       // Enable 50% tune duty cycle when = 1

/*****************************************
       Winkey xmode2 byte
 ****************************************/

const quint8 PDLSTAT =   0x80;       // Disable paddle status updates = 1
const quint8 FASTCMD =   0x40;       // Enable fast command response when = 1
const quint8 CUT_9 =     0x20;       // Cut 9 enable
const quint8 CUT_0 =     0x10;       // Cut 0 enable

const quint8 PDLTONE =   0x08;       // Paddle only sidetone when = 1
const quint8 SO2R =      0x04;       // SO2R mode enabled when = 1
const quint8 PDLMUTE =   0x02;       // Paddle mute when = 1

/*****************************************
       WKrtty mode byte
 ****************************************/

const quint8 RTTY_ON =   0x80;       // Enable FSK mode when = 1
const quint8 DIDDLE =    0x40;       // Enable blank stream when = 1
const quint8 FSKMAP =    0x20;       // Swap PTT & KEY outputs
const quint8 AUTOCRLF =  0x10;       // Automatic CRLF at line overflow
const quint8 RYMON =     0x08;       // Enable FSK echo when = 1
const quint8 REVERSE =   0x04;       // Revers mark/space sense when = 1

/*****************************************
       RYmode byte
 ****************************************/

const quint8 RYIGNORE =  0x20;       // Ignore non-Baudot when = 0, else print ?
const quint8 RYASR =     0x10;       // RTTY Auto Send/Receive when = 1
const quint8 RYSTOP =    0x08;       // 2 stop bits when=0, 1.5 stop bits when = 1
const quint8 DIDLCHAR =  0x04;       // Diddle=Blank when 0, Diddle=Ltrs when 1
const quint8 USOSMODE =  0x01;       // Set robust USOS mode when = 1

/****************************************
       Winkey pot value
 ***************************************/

// Bits 4-0  specify current pot value (0-31)


/****************************************
       Paddle Direct Defines
 ***************************************/

const quint8 DAH_DIRECT =      1;
const quint8 NDAH_DIRECT =     0xFE;
const quint8 DIT_DIRECT =      2;
const quint8 NDIT_DIRECT =     0xFD;

const quint8 MAX_TONE =        0x0B;
const quint8 MIN_TONE =        0x00;


/*******************************************
*	Command Equates Table
*******************************************/



/* Immediate Commands */
const quint8 CMD_END = 0x00;
const quint8 ADMIN_CMD = 0x00;     // Administration Command
const quint8 FREQ_CMD =  0x01;     // Set sidetone frequency to NN
const quint8 SPEED_CMD = 0x02;     // Set keying speed to NN
const quint8 WEIGHT_CMD =  0x03;     // Weight modify to NN
const quint8 LEADTAIL_CMD = 0x04;     // XMIT Lead-In/Tail NN NN
const quint8 WPMRANGE_CMD = 0x05;     // WPM Range NN NN
const quint8 PAUSE_CMD =    0x06;     // Pause transmit On/Off
const quint8 GETPOT_CMD =   0x07;     // Return WK's speed pot value
const quint8 BACKSP_CMD =   0x08;     // Backspace buffer input pointer
const quint8 PINCFG_CMD =   0x09;     // Setup Output pin configuration
const quint8 CLRBUF_CMD =   0x0A;     // Clear circular buffer
const quint8 KEYIMM_CMD =   0x0B;     // Key immediately
const quint8 HSCW_CMD =     0x0C;     // HSCW speed set
const quint8 SETFARNS_CMD = 0x0D;     // Set Farnsworth WPM
const quint8 MODE_CMD =     0xE;      // Set mode register direct
const quint8 DFLTS_CMD =    0x0F;     // Load default register block
const quint8 XTND_CMD =     0x10;     // First bit extension
const quint8 KCOMP_CMD =    0x11;     // Keying Compensation
const quint8 SAMPADJ_CMD =  0x12;     // Adjust swing sensitivity
const quint8 NULLIMM_CMD =  0x13;     // Null immediate command
const quint8 DIRECTKEY_CMD = 0x14;    // Wake up winkey from hard reset
const quint8 GETSTAT_CMD =  0x15;     // Return winkey's status byte
const quint8 POINTER_CMD =  0x16;     // Get/Set input buffer pointer
const quint8 DUTY_CMD =     0x17;     // Set dit/dah duty cycle

/* Buffered Message Commands */

const quint8 FIRST_BUF_CMD  = 0x18;

const quint8 BUFPTT_CMD =  0x18;     // PTT Control
const quint8 BUFKEY_CMD =  0x19;     // Key Down Control
const quint8 WAIT_CMD   =  0x1A;     // Wait for NN secs
const quint8 MERGE_CMD =   0x1B;     // Merge two
const quint8 BUFSPEED_CMD =	0x1C;     // Change speed (buffered)
const quint8 BUFHSCW_CMD =   0x1D;     // Buffered HSCW speed set
const quint8 CANCELSPD_CMD = 0x1E;     // Cancel buffered speed
const quint8 NULLBUF_CMD =  0x1F;     // Unassigned

const quint8 CMD_TBL_END =  0x1F;

/****************************************
       ADMIN Commands
 ***************************************/

#define	ADMIN_CAL       0
#define ADMIN_RESET     1
#define ADMIN_OPEN      2
#define ADMIN_CLOSE     3
#define ADMIN_ECHO      4
#define ADMIN_A2DPDL	5
#define ADMIN_A2DPOT	6
#define ADMIN_STATE     7
#define ADMIN_DEBUG     8
#define ADMIN_GETVERS   9
#define ADMIN_SETWK1   10
#define ADMIN_SETWK2   11
#define ADMIN_DUMPEEP  12
#define ADMIN_LOADEEP  13
#define ADMIN_SENDMSG  14
#define ADMIN_LDX1MODE 15
#define ADMIN_UPDATE   16
#define ADMIN_LOBAUD   17
#define ADMIN_HIBAUD   18
#define ADMIN_SETRTTY  19
#define ADMIN_SETWK3   20
#define ADMIN_RDVOLTS  21
#define ADMIN_LDX2MODE 22
#define ADMIN_GETMINOR 23
#define ADMIN_GETTYPE  24
#define ADMIN_SETVOL   25
#define ADMIN_NONE 255

/****************************************
 *      Winkey Defaults
 ***************************************/

#define DEFAULT_COMPORT     ""
#define DEFAULT_BAUDRATE    "1200"
#define DEFAULT_WEIGHT      50
#define DEFAULT_LEADIN      00
#define DEFAULT_TAIL        00
#define DEFAULT_XTND        00
#define DEFAULT_KCOMP       00
#define DEFAULT_FARNS       00
#define DEFAULT_SAMPADJ     00
#define DEFAULT_DITDAHRATIO 50
#define DEFAULT_PINCFG      USESTONE+KEYPORT0
#define DEFAULT_MINWPM      5
#define DEFAULT_WPMRANGE    25
#define DEFAULT_STCONST     (62500/752)
#define DEFAULT_STFREQ      800
#define DEFAULT_SPEED       16
#define DEFAULT_MODEREG     00
#define DEFAULT_X1MODE      00
#define DEFAULT_X2MODE      00
#define DEFAULT_WKRTTY      00
#define DEFAULT_RYMODE      00
#define DEFAULT_VOLUME      4
#define DEFAULT_MESSAGE     ""

/****************************************
       Winkey Manager Return Codes
 ***************************************/

#define WK_SUCCESS               0
#define NO_ECHO_RESPONSE        -1
#define ECHO_MISMATCH           -2
#define NO_OPEN_RESPONSE        -3
#define COM_OPEN_FAILED         -4
#define COM_OPEN_OK		 0
#define GET_COM_STATE_FAILED    -5
#define SET_COM_STATE_FAILED    -6
#define WK_ALREADY_OPEN         -8
#define WK_NOT_OPEN             -9
#define COM_PURGE_FAILED       -10

const quint8 KBD_BACKSPACE = 0x08;
const quint8 KBD_CR = 0x0d;
const quint8 KBD_ESC = 0x1b;

QString WINKEYER_PATH_LOGGER();
const QString WINKEYER_CONFIG_FILENAME = "winkeyerConfig.ini";

const QStringList adminCmdNames = {"CAL", "RESET", "OPEN", "CLOSE",
                                    "ECHO", "A2DPDL", "A2DPOT",
                                    "STATE", "DEBUG", "GETVERS",
                                    "SETWK1", "SETWK2", "DUMPEEP",
                                    "LOADEEP", "SENDMSG", "LDX1MODE",
                                    "UPDATE", "LOBAUD", "HIBAUD",
                                    "SETRTTY", "SETWK3", "RDVOLTS",
                                   "LDX2MODE", "GETMINOR", "GETTYPE", "SETVOL"};

// error codes
const int SERIAL_OK = 0;
const int SERIALPORT_FAILED_TO_OPEN = -1;
const int SERIAL_PORT_EMPTY = -2;


const QStringList keyerModeListStr = QStringList() << "Iambic A" << "Iambic B" << "Ultimate" << "UltimDah" << "UltimDih" << "Vibrobug";
const QStringList sidetoneListStr = QStringList() << "3759" << "1879" << "1252" << "940"
                                                  << "752" << "625" << "535" << "469"
                                                  << "417" << "375";
const QStringList hangtimeListStr = QStringList() << "1.0 Word" << "1.3 Word" << "1.6 Word" << "2.0 Word";

class WinkeyerState
{
public:

    WinkeyerState()
    {
        setDefaults();
    }

    WinkeyerState(const WinkeyerState &other);

    WinkeyerState& operator=(const WinkeyerState &other);

    void setModereg(quint8 modereg_)
    {
        modereg = modereg_;
    }
    quint8 getModereg()
    {
        return modereg;
    }





    void setSpeed(quint8 speed_)
    {
        speed = speed_;
    }
    quint8 getSpeed()
    {
        return speed;
    }

    void setStconst(quint8 stconst_)
    {
        stconst = stconst_;
    }
    quint8 getStconst()
    {
        return stconst;
    }

    void setWeight(quint8 weight_)
    {
        weight = weight_;
    }
    quint8 getWeight()
    {
        return weight;
    }

    void setLeadin(quint8 leadin_)
    {
        leadin = leadin_;
    }
    quint8 getLeadin()
    {
        return leadin;
    }

    void setTail(quint8 tail_)
    {
        tail = tail_;
    }
    quint8 getTail()
    {
        return tail;
    }

    void setMinwpm(quint8 minwpm_)
    {
        minwpm = minwpm_;
    }
    quint8 getMinwpm()
    {
        return minwpm;
    }

    void setWpmrange(quint8 wpmrange_)
    {
        wpmrange = wpmrange_;
    }
    quint8 getWpmrange()
    {
        return wpmrange;
    }

    void setXtnd(quint8 xtnd_)
    {
        xtnd = xtnd_;
    }
    quint8 getXtnd()
    {
        return xtnd;
    }

    void setKcomp(quint8 kcomp_)
    {
        kcomp = kcomp_;
    }
    quint8 getKcomp()
    {
        return kcomp;
    }

    void setFarns(quint8 farns_)
    {
        farns = farns_;
    }
    quint8 getFarns()
    {
        return farns;
    }

    void setSampadj(quint8 sampadj_)
    {
        sampadj = sampadj_;
    }
    quint8 getSampadj()
    {
        return sampadj;
    }

    void setDitdahratio(quint8 ditdahratio_)
    {
        ditdahratio = ditdahratio_;
    }
    quint8 getDitdahratio()
    {
        return ditdahratio;
    }

    void setPincfg(quint8 pincfg_)
    {
        pincfg = pincfg_;
    }
    quint8 getPincfg()
    {
        return pincfg;
    }

    void setX1mode(quint8 x1mode_)
    {
        x1mode = x1mode_;
    }
    quint8 getX1mode()
    {
        return x1mode;
    }

    void setDefaults();



private:

    void copy(const WinkeyerState &other);


    quint8 modereg;
    quint8 speed;
    quint8 stconst;
    quint8 weight;
    quint8 leadin;
    quint8 tail;
    quint8 minwpm;
    quint8 wpmrange;
    quint8 xtnd;
    quint8 kcomp;
    quint8 farns;
    quint8 sampadj;
    quint8 ditdahratio;
    quint8 pincfg;
    quint8 x1mode;






};


class WinkeyerStateStorage
{
public:

    WinkeyerStateStorage(){}

    WinkeyerStateStorage(const WinkeyerStateStorage &other)
    {
        copy(other);
    }

    WinkeyerStateStorage& operator=(const WinkeyerStateStorage &other)
    {
        if (this != &other) {
            copy(other);
        }
        return *this;
    }

    void copy(const WinkeyerStateStorage &other)
    {
        comport = other.comport;
        baudrate = other.baudrate;
        wkState = QSharedPointer<WinkeyerState>::create(*other.wkState);
        x2mode = other.x2mode;
        WKrtty = other.WKrtty;
        RYmode = other.RYmode;
        STvolume = other.STvolume;
        msg1Buf = other.msg1Buf;
        msg2Buf = other.msg2Buf;
        msg3Buf = other.msg3Buf;
        msg4Buf = other.msg4Buf;
        msg5Buf = other.msg5Buf;
        msg6Buf = other.msg6Buf;
    }

    void setComport(const QString comport_)
    {
        comport = comport_;
    }
    QString getComport()
    {
        return comport;
    }

    void setBaudrate(const QString baudrate_)
    {
        baudrate = baudrate_;
    }
    QString getBaudrate()
    {
        return baudrate;
    }

    void setWkstate(QSharedPointer<WinkeyerState> wkState_)
    {
        wkState = wkState_;
    }
    QSharedPointer<WinkeyerState> getWkState()
    {
        return wkState;
    }

    void setX2mode(const quint8 x2mode_)
    {
        x2mode = x2mode_;
    }
    quint8 getX2mode()
    {
        return x2mode;
    }

    void setWKrtty(const quint8 WKrtty_)
    {
        WKrtty = WKrtty_;
    }
    quint8 getWKrtty()
    {
        return WKrtty;
    }

    void setRYmode(const quint8 RYmode_)
    {
        RYmode = RYmode_;
    }
    quint8 getRYmode()
    {
        return RYmode;
    }

    void setSTvolume(const quint8 STvolume_)
    {
        STvolume = STvolume_;
    }
    quint8 getSTvolume()
    {
        return STvolume;
    }

    void setMsgBuf(const QByteArray &msg, const int bufNum)
    {

        switch(bufNum)
        {
        case 1:
            msg1Buf = msg;
            break;
        case 2:
            msg2Buf = msg;
            break;
        case 3:
            msg3Buf = msg;
            break;
        case 4:
            msg4Buf = msg;
            break;
        case 5:
            msg5Buf = msg;
            break;
        case 6:
            msg6Buf = msg;
            break;
        default:
            qDebug() << "Incorrect buffer number";
        }

    }

    QByteArray getMsgBuf(const int bufNum)
    {

        switch(bufNum)
        {
        case 1:
            return msg1Buf;
            break;
        case 2:
            return msg2Buf;
            break;
        case 3:
            return msg3Buf;
            break;
        case 4:
            return msg4Buf;
            break;
        case 5:
            return msg5Buf;
            break;
        case 6:
            return msg6Buf;
            break;
        default:
            qDebug() << "Incorrect buffer number";
        }

        return nullptr; // check before using!

    }





    void loadWinkeyerStateStorageFromFile(QSettings& config)
    {
        config.beginGroup("winkeyerstate");

        setComport(config.value("comport", DEFAULT_COMPORT).toString());
        setBaudrate(config.value("baudrate", DEFAULT_BAUDRATE).toString());
        getWkState()->setModereg(static_cast<quint8>(config.value("modeReg", static_cast<int>(DEFAULT_MODEREG)).toInt()));
        getWkState()->setSpeed(static_cast<quint8>(config.value("speed", static_cast<int>(DEFAULT_SPEED)).toInt()));
        getWkState()->setStconst(static_cast<quint8>(config.value("stconst", static_cast<int>(DEFAULT_STCONST)).toInt()));
        getWkState()->setWeight(static_cast<quint8>(config.value("weight", static_cast<int>(DEFAULT_WEIGHT)).toInt()));
        getWkState()->setLeadin(static_cast<quint8>(config.value("leadin", static_cast<int>(DEFAULT_LEADIN)).toInt()));
        getWkState()->setTail(static_cast<quint8>(config.value("tail", static_cast<int>(DEFAULT_TAIL)).toInt()));
        getWkState()->setMinwpm(static_cast<quint8>(config.value("minwpm", static_cast<int>(DEFAULT_MINWPM)).toInt()));
        getWkState()->setWpmrange(static_cast<quint8>(config.value("wpmrange", static_cast<int>(DEFAULT_WPMRANGE)).toInt()));
        getWkState()->setXtnd(static_cast<quint8>(config.value("xtnd", static_cast<int>(DEFAULT_XTND)).toInt()));
        getWkState()->setKcomp(static_cast<quint8>(config.value("kcomp", static_cast<int>(DEFAULT_KCOMP)).toInt()));
        getWkState()->setFarns(static_cast<quint8>(config.value("farns", static_cast<int>(DEFAULT_FARNS)).toInt()));
        getWkState()->setSampadj(static_cast<quint8>(config.value("sampadj", static_cast<int>(DEFAULT_SAMPADJ)).toInt()));
        getWkState()->setDitdahratio(static_cast<quint8>(config.value("ditdahratio", static_cast<int>(DEFAULT_DITDAHRATIO)).toInt()));
        getWkState()->setPincfg(static_cast<quint8>(config.value("pincfg", static_cast<int>(DEFAULT_PINCFG)).toInt()));
        getWkState()->setX1mode(static_cast<quint8>(config.value("x1mode", static_cast<int>(DEFAULT_X1MODE)).toInt()));
        setX2mode(static_cast<quint8>(config.value("x2Mode", static_cast<int>(DEFAULT_X2MODE)).toInt()));
        setWKrtty(static_cast<quint8>(config.value("wKrtty", static_cast<int>(DEFAULT_WKRTTY)).toInt()));
        setRYmode(static_cast<quint8>(config.value("rymode", static_cast<int>(DEFAULT_RYMODE)).toInt()));
        setSTvolume(static_cast<quint8>(config.value("sideToneVolume", static_cast<int>(DEFAULT_VOLUME)).toInt()));
        setMsgBuf(config.value("msg2Buf", DEFAULT_MESSAGE).toString().toUtf8(), 1);
        setMsgBuf(config.value("msg2Buf", DEFAULT_MESSAGE).toString().toUtf8(), 2);
        setMsgBuf(config.value("msg2Buf", DEFAULT_MESSAGE).toString().toUtf8(), 3);
        setMsgBuf(config.value("msg2Buf", DEFAULT_MESSAGE).toString().toUtf8(), 4);
        setMsgBuf(config.value("msg2Buf", DEFAULT_MESSAGE).toString().toUtf8(), 5);
        setMsgBuf(config.value("msg2Buf", DEFAULT_MESSAGE).toString().toUtf8(), 6);

        config.endGroup();


    }


    void saveWinkeyerStateStorageToFile(QSettings& config)
    {

        config.beginGroup("winkeyerstate");
        config.setValue("comport",  getComport());
        config.setValue("baudrate",  getBaudrate());
        config.setValue("modereg", static_cast<int>(getWkState()->getModereg()));
        config.setValue("speed", static_cast<int>(getWkState()->getSpeed()));
        config.setValue("stconst", static_cast<int>(getWkState()->getStconst()));
        config.setValue("weight", static_cast<int>(getWkState()->getWeight()));
        config.setValue("leadin", static_cast<int>(getWkState()->getLeadin()));
        config.setValue("tail", static_cast<int>(getWkState()->getTail()));
        config.setValue("minwpm", static_cast<int>(getWkState()->getMinwpm()));
        config.setValue("wpmrange", static_cast<int>(getWkState()->getWpmrange()));
        config.setValue("xtnd", static_cast<int>(getWkState()->getXtnd()));
        config.setValue("kcomp", static_cast<int>(getWkState()->getKcomp()));
        config.setValue("farns", static_cast<int>(getWkState()->getFarns()));
        config.setValue("sampadj", static_cast<int>(getWkState()->getSampadj()));
        config.setValue("ditdahratio", static_cast<int>(getWkState()->getDitdahratio()));
        config.setValue("pincfg", static_cast<int>(getWkState()->getPincfg()));
        config.setValue("x1mode", static_cast<int>(getWkState()->getX1mode()));
        config.setValue("x2Mode", static_cast<int>(getX2mode()));
        config.setValue("wKrtty", static_cast<int>(getWKrtty()));
        config.setValue("rymode", static_cast<int>(getRYmode()));
        config.setValue("sideToneVolume", static_cast<int>(getSTvolume()));
        QVariant msg = getMsgBuf(1);
        config.setValue("msg1Buf", msg);
        msg = getMsgBuf(2);
        config.setValue("msg2Buf", msg);
        msg = getMsgBuf(3);
        config.setValue("msg3Buf", msg);
        msg = getMsgBuf(4);
        config.setValue("msg4Buf", msg);
        msg = getMsgBuf(5);
        config.setValue("msg5Buf", msg);
        msg = getMsgBuf(6);
        config.setValue("msg6Buf", msg);
        config.endGroup();




    }






private:

    QString comport;
    QString baudrate;
    QSharedPointer<WinkeyerState> wkState;
    quint8 x2mode;
    quint8 WKrtty;
    quint8 RYmode;
    quint8 STvolume;
    QByteArray msg1Buf;
    QByteArray msg2Buf;
    QByteArray msg3Buf;
    QByteArray msg4Buf;
    QByteArray msg5Buf;
    QByteArray msg6Buf;



};

#endif // WINKEYERCOMMON_H
