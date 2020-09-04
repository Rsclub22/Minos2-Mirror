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

QString convertFreqStrDisp(QString sfreq)
{

    if (sfreq == "")
    {
        return sfreq;
    }

    if (sfreq.contains('.'))
    {
        sfreq.remove('.');
    }

    int len = sfreq.length();

    switch(len)
    {
        case 11:
            sfreq = sfreq.insert(8, '.');
            sfreq = sfreq.insert(5, '.');
            sfreq = sfreq.insert(2, '.');
            break;
        case 10:
            sfreq = sfreq.insert(7, '.');
            sfreq = sfreq.insert(4, '.');
            sfreq = sfreq.insert(1, '.');
            break;
        case 9:
            sfreq = sfreq.insert(3, '.');
            sfreq = sfreq.insert(7, '.');
            break;
        case 8:
            sfreq = sfreq.insert(2, '.');
            sfreq = sfreq.insert(6, '.');
            break;
        case 7:
            sfreq = sfreq.insert(4, '.');
            sfreq = sfreq.insert(1, '.');
            break;
        case 6:
            sfreq = sfreq.insert(3,'.');
            break;
        case 5:
            sfreq = sfreq.insert(2,'.');
            break;
        case 4:
            sfreq = sfreq.insert(1,'.');
            break;
        default:
            sfreq = "00.000.000.000";    // error

    }


    return sfreq;
}




QString convertFreqToStr(double frequency)
{

    return QString::number(frequency,'f', 0);

}


QString convertFreqToStr(int frequency)
{

    return QString::number(frequency, 10);

}

QString convertFreqToStr(qint64 frequency)
{
    return QString::number(frequency, 10);
}

QString convertFreqToStr(quint64 frequency)
{
    return QString::number(frequency, 10);
}


double convertStrToFreq(QString frequency)
{
    bool ok = false;
    double f = 0.0;
    f = frequency.toDouble(&ok);
    if (ok)
    {
        return f;
    }
    else
    {
        return -1.0;
    }
}












/*
QString validateFreqTxtInput(QString f, bool* ok)
{

    if (f == "")
    {
        *ok = true;
        return f;
    }


    QRegularExpression f1rx = QRegularExpression("\\d{1,5}\\.\\d{3,6}");  // match ghz_mhz.khz_hz

    QString retFreq = "";
    if (f.count('.') != 1)
    {
        *ok = false;
        return f;
    }


    if (f1rx.exactMatch(f))
    {
        QStringList sl = f.split('.');
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

        *ok = true;
    }
    else
    {
        // error
        *ok = false;
    }


    return retFreq;
}
*/

/* Single Delimiter Utilities */

// convert freq string for display - single delimter
// 144.000000

QString convertFreqStrDispSingle(QString sfreq)
{

    int len = sfreq.length();

    switch(len)
    {
        case 11:
            //sfreq = sfreq.insert(8, '.');
            sfreq = sfreq.insert(5, '.');
            //sfreq = sfreq.insert(2, '.');
            break;
        case 10:
            //sfreq = sfreq.insert(7, '.');
            sfreq = sfreq.insert(4, '.');
            //sfreq = sfreq.insert(1, '.');
            break;
        case 9:
            sfreq = sfreq.insert(3, '.');
            //sfreq = sfreq.insert(7, '.');
            break;
        case 8:
            sfreq = sfreq.insert(2, '.');
            //sfreq = sfreq.insert(6, '.');
            break;
        case 7:
            //sfreq = sfreq.insert(4, '.');
            sfreq = sfreq.insert(1, '.');
            break;
        case 6:
            sfreq = sfreq.insert(0,'.');
            break;
        //case 5:
        //    sfreq = sfreq.insert(2,'.');
        //    break;
        //case 4:
        //    sfreq = sfreq.insert(1,'.');
        //    break;
        default:
            sfreq = "0.0";    // error

    }

    return sfreq;

}


QString convertFreqStrDispSingleNoTrailZero(QString sfreq)
{

        int len = sfreq.length();

        switch(len)
        {
            case 11:
                //sfreq = sfreq.insert(8, '.');
                sfreq = sfreq.insert(5, '.');
                //sfreq = sfreq.insert(2, '.');
                break;
            case 10:
                //sfreq = sfreq.insert(7, '.');
                sfreq = sfreq.insert(4, '.');
                //sfreq = sfreq.insert(1, '.');
                break;
            case 9:
                sfreq = sfreq.insert(3, '.');
                //sfreq = sfreq.insert(7, '.');
                break;
            case 8:
                sfreq = sfreq.insert(2, '.');
                //sfreq = sfreq.insert(6, '.');
                break;
            case 7:
                //sfreq = sfreq.insert(4, '.');
                sfreq = sfreq.insert(1, '.');
                break;
            case 6:
                sfreq = sfreq.insert(0,'.');
                break;
            //case 5:
            //    sfreq = sfreq.insert(2,'.');
            //    break;
            //case 4:
            //    sfreq = sfreq.insert(1,'.');
            //    break;
            default:
                sfreq = "??.???.???.???";    // error

        }

        sfreq = removeTrailingZeroes(sfreq);
        return sfreq;
}




QString removeTrailingZeroes(QString sfreq)
{
    // remove trailing zero, apart from after period.
    QStringList fspl = sfreq.split('.');
    int last = fspl.count() - 1;
    fspl[last].remove(QRegularExpression("0+$"));  //remove trailing zeros
    if (fspl[last].count() == 0)
    {
        fspl[last] = "0";    // add back one zero
    }

    sfreq = fspl.join(".");

    return sfreq;
}


bool validateFreqTxtInput(QString f)
{

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

// converts freq received from cluster from khz to ghz and mhz
// not used...
QString convertKhzToMhz(QString f)
{

    QStringList sl;
    if (!f.contains('.'))
    {
        return f;
    }
    else
    {

        sl = f.split('.');
        if (sl.count() == 2)
        {
            if (sl[0].count() <= 6 && sl[0].count() > 3)
            {
                sl[0] = sl[0].insert(sl[0].count() - 3, '.');
            }
            else
            {
                if (sl[0].count() >= 4 && sl[0].count() <= 8)
                {
                    sl[0] = sl[0].insert(sl[0].count() - 6, '.');
                    sl[0] = sl[0].insert(sl[0].count() - 3, '.');

                }
            }
        }
    }



    return sl[0];
}


// used to pad display of freq with spaces

QString alignFreqRight(QString f)
{
    QString freq = f;
    QStringList sl;
    if (freq.count('.') == 1)
    {
        sl = freq.split('.');
        if (sl.count() == 2)
        {
            for (int i = 0; i < (6 - sl[0].count()); i++)
            {
                freq.prepend('\x20');
            }
            return freq;
        }
    }
    else if (freq.count('.') == 2)
    {
        sl = freq.split('.');
        if (sl.count() == 3)
        {
            if (sl[0].count() == 1)
            {
                freq.prepend('\x20');
            }
            return freq;
        }
    }

    return freq;
}


QString convertSinglePeriodFreqToMultiPeriod(QString f)
{

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

QString convertRitFreqToStr(int freq, bool ritKHzFlag)
{

    bool negNum = false;

    QString rfreq = convertFreqToStr(freq);




    if (rfreq[0] == '-')
    {
        negNum = true;
        rfreq = rfreq.remove('-');
        freq = freq * -1;
    }

    if (ritKHzFlag)
    {
        if (freq < 9)
        {
            rfreq = QString("+00.00");
            return rfreq;
        }

        if (freq < 1000)
        {
            rfreq.prepend("00.");
        }
        else if (freq < 10000)
        {
            rfreq.insert(1, '.').prepend('0');

        }
        else if (freq >= 10000)
        {
            rfreq.insert(2, '.');
        }
    }
    else
    {
        if (freq < 9)
        {
            rfreq = QString("+0.00");
            return rfreq;
        }
        if (freq < 1000)
        {
            rfreq.prepend(("0."));
        }
        if (freq >= 1000)
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



/*
// modified for tens khz
// if tensKHz is true - support +/- 99KHz
// false support +/- 9KHz

QString convertRitFreqToStr(int freq, bool ritKHzFlag)
{

    bool negNum = false;
    if (freq == 0 && freq <= 9)  // not interested in Hz
    {
        if (ritKHzFlag)
        {
           return QString("+00.00");
        }
        else
        {
            return QString ("+0.00");
        }

    }

    QString rfreq = convertFreqToStr(freq);


    if (rfreq[0] == '-')
    {
        negNum = true;
        rfreq = rfreq.remove('-');
    }

    if (ritKHzFlag)
    {
        if (freq < 1000)
        {
            rfreq.prepend("00.");
        }
        else if (freq < 10000)
        {
            rfreq.insert(1, '.').prepend('0');

        }
        else if (freq >= 10000)
        {
            rfreq.insert(2, '.');
        }
    }
    else
    {
        if (freq < 1000)
        {
            rfreq.prepend(("0."));
        }
        if (freq >= 1000)
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


    return rfreq;

}
*/

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

QString extractKhz(QString f)
{
    QString khz = "***";
    QString sf = f.remove('.');
    bool ok = false;
    double df = sf.toDouble(&ok);
    if (ok && df != 0.0)
    {
        if (f.contains('.'))
        {
            QStringList k = f.split('.');
            int i = k.length();
            if (i >=2)
            {
                return k[i-2];
            }
        }
        else
        {
            int i = f.length();
            if (i >= 6)
            {
                khz = f.mid(i - 6, 3);
                return khz;
            }
        }
    }


    return khz;


}



