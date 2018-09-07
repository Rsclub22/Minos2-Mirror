/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) D G Balharrie M0DGB/G8FKH
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include "rigutils.h"

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


    QRegExp f1rx = QRegExp("\\d{1,5}\\.\\d{3,6}");  // match ghz_mhz.khz_hz

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
    fspl[last].remove(QRegExp("0+$"));  //remove trailing zeros
    if (fspl[last].count() == 0)
    {
        fspl[last] = "0";    // add back one zero
    }

    sfreq = fspl.join(".");

    return sfreq;
}


bool validateFreqTxtInput(QString f)
{

    //QRegExp f1rx = QRegExp("\\d{1,5}\\.\\d{3,6}");  // match ghz_mhz.khz_hz
    QRegExp f1rx = QRegExp("\\d{1,5}\\.\\d{1,6}");  // match ghz_mhz.khz_hz
    if (f1rx.exactMatch(f))
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





QString convertRitFreqToStr(double freq)
{

    bool negNum = false;
    if (freq == 0.0)
    {
        return QString("+0.00");
    }

    QString rfreq = convertFreqToStr(freq);


    if (rfreq[0] == '-')
    {
        negNum = true;
        rfreq = rfreq.remove('-');
    }

    if (rfreq.count() == 2)
    {
        rfreq = QString("0.0" + rfreq).left(4);
    }
    else if (rfreq.count() == 3)
    {
        rfreq = QString("0." + rfreq).left(4);
    }
    else if (rfreq.count() == 4)
    {
        rfreq = rfreq.insert(1, '.').left(4);
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


