/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) D G Balharrie M0DGB/G8FKH
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include "cutils.h"
#include "rigutils.h"

// I've duplicated the strings to make their translation easier.

const char * FREQ_EDIT_ERR_MSG = QT_TRANSLATE_NOOP("FrequencyDisplay", "Frequency has invalid characters or missing periods.\r\n\r\nThe format required is (e.g.) 1296.3, 144.32 or 144.290123\r\n");
const char * RADIO_FREQ_EDIT_ERR_MSG = QT_TRANSLATE_NOOP("FrequencyDisplay", "Radio Frequency has invalid characters or missing periods.\r\n\r\nThe format required is (e.g.) 1296.3, 144.32 or 144.290123\r\n");
const char * TARGET_FREQ_EDIT_ERR_MSG = QT_TRANSLATE_NOOP("FrequencyDisplay", "Target Frequency has invalid characters or missing periods.\r\n\r\nThe format required is (e.g.) 1296.3, 144.32 or 144.290123\r\n");

// add delimiter to string for display
// input string should just be digits


bool validateFreqTxtInput(QString f)
{

    if (f.remove('.') == "000014400")
    {
        trace("000014400");
    }
    //QRegularExpression f1rx = QRegularExpression("\\d{1,5}\\.\\d{3,6}");  // match ghz_mhz.khz_hz
    QRegularExpression f1rx = QRegularExpression(anchoredPattern("\\d{1,5}\\.\\d{1,6}"));  // match ghz_mhz.khz_hz
    QRegularExpressionMatch f1rxm = f1rx.match(f);
    if (f1rxm.hasMatch())
    {
        return true;
    }
    else
    {

        // error
        return false;
    }


}


bool valInputFreq(QString f, QString errMsg)
{
    if (f.remove('.') == "000014400")
    {
        trace("000014400");
    }

    if (f == "")
    {
        return false;
    }

    if (!f.contains('.'))
    {
        f = f + "." + "000000";
    }
    else if (f.count('.') == 1)
    {
        QStringList fl = f.split('.');
        fl[1] = fl[1] + "000000";
        fl[1].truncate(6);
        f = fl[0] + "." + fl[1];
    }
    if (!validateFreqTxtInput(f))
    {
        // error
        QMessageBox msgBox;
        msgBox.setText(errMsg);
        msgBox.exec();
        return false;             //incorrect format
    }

    return true;



}

// This will convert "144.3" etc to "144.300000"

QString convertSinglePeriodFreqToFullDigit(QString f)
{
    if (f.remove('.') == "000014400")
    {
        trace("000014400");
    }

    if (f.contains('.'))
    {
        QStringList fl = f.split('.');
        fl[1] = fl[1] + "000000";
        fl[1].truncate(6);
        return fl[0] + "." + fl[1];
    }

    return f;

}


QString convertFreqToFullDigit(QString f)
{
    if (f.remove('.') == "000014400")
    {
        trace("000014400");
    }


    if (f.contains('.'))
    {
        QStringList fl = f.split('.');
        fl[1] = fl[1] + "000000";
        fl[1].truncate(6);
        return fl[0] + "." + fl[1];
    }
    else
    {
        f = f + "." + "000000";
    }

    return f;



}

QString convertSinglePeriodFreqToMultiPeriod(QString f)
{
    if (f.remove('.') == "000014400")
    {
        trace("000014400");
    }

    QString retFreq = "";

    if (f == "")
    {
        return retFreq;
    }

    QStringList sl = f.split('.');
    if (sl.count() == 1)
    {
        sl.append("000");
    }
    if (sl[0].count() > 3)
    {
        retFreq = sl[0].left(sl[0].count()-3) + "." + sl[0].right(3) + ".";
    }
    else
    {
        retFreq = sl[0] + ".";
    }

    if (sl[1].count() > 3)
    {
       retFreq = retFreq + sl[1].left(3) + "." + sl[1].right(sl[1].count()-3);
       if (sl[1].count() == 4)
       {
           retFreq = retFreq + "00";
       }
       else if (sl[1].count() == 5)
       {
           retFreq = retFreq + "0";
       }
    }
    else
    {
       retFreq = retFreq + sl[1] + "." + "000";
    }


    return retFreq;


}



// modified for tens khz
// if tensKHz is true - support +/- 99KHz
// false support +/- 9KHz

QString convertRitFreqToStr(const ShortFreq &freq, bool ritKHzFlag)
{

    bool negNum = false;

    QString rfreq = freq.str();

    int f = qint32(freq);

    if (rfreq[0] == '-')
    {
        negNum = true;
        rfreq = rfreq.remove('-');
        f = f * -1;
    }

    if (ritKHzFlag)
    {
        if (f < 9)
        {
            rfreq = QString("+00.00");
            return rfreq;
        }

        if (f < 1000)
        {
            rfreq.prepend("00.");
        }
        else if (f < 10000)
        {
            rfreq.insert(1, '.').prepend('0');

        }
        else if (f >= 10000)
        {
            rfreq.insert(2, '.');
        }
    }
    else
    {
        if (f < 9)
        {
            rfreq = QString("+0.00");
            return rfreq;
        }
        if (f < 1000)
        {
            rfreq.prepend(("0."));
        }
        if (f >= 1000)
        {
            rfreq.insert(1, '.');
        }

    }


    if (negNum)
    {
        rfreq = rfreq.prepend('-');
    }
    else
    {
        rfreq = rfreq.prepend('+');
    }

    if (ritKHzFlag)
    {

        rfreq = rfreq.mid(0,6);  // get rid of tens digit
    }
    else
    {
        rfreq = rfreq.mid(0,5);  // get rid of tens digit
    }

    return rfreq;

}

// remove hundreds hz and hz from freq for cluster display
QString removeHundredHzAndHzDigits(QString f)
{
    QStringList sl = f.split('.');
    int count = sl.count();
    sl[count - 1] = sl[count - 1].mid(0,1);
    QString fnew;
    for (int i = count - 1; i >= 0; i--)
    {
        if (i == count - 1)
        {
            fnew = (sl[i]);
        }
        else
        {
            fnew.prepend(sl[i] + ".");
        }

    }
    return fnew;
}





