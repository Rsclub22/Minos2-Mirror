/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include "base_pch.h"
#include "rigutils.h"
#include "freqlineedit.h"

FreqLineEdit::FreqLineEdit(QWidget *parent):
    QLineEdit(parent)
{

}


FreqLineEdit::~FreqLineEdit()
{


}




void FreqLineEdit::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numTicks = numDegrees / 15;

    if (numTicks == 1)
    {
       changeFreq(true);
    }
    else
    {
        changeFreq(false);
    }

    event->accept();
}


void FreqLineEdit::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_Up)
    {
        changeFreq(true);
    }
    else if(event->key() == Qt::Key_Down)
    {
        changeFreq(false);

    }
    else if (event->key() == Qt::Key_Return)
    {
        emit freqEditReturn();
        return;
    }
    else
    {
        // default handler for event
        QLineEdit::keyPressEvent(event);
    }
}



void FreqLineEdit::changeFreq(bool direction)
{
    static const double tuningData[][14] =
                                        {
                                        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {100, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {1000, 0, 100, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {10000, 1000, 0, 100, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {100000, 10000, 1000, 0, 100, 10, 1, 0, 0, 0, 0, 0, 0, 0},
                                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1, 0, 0, 0, 0, 0},
                                        {10000000, 1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1, 0, 0, 0, 0},
                                        {100000000, 10000000, 1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1, 0, 0, 0},
                                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {1000000000, 0, 100000000, 10000000, 1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1, 0},
                                        {10000000000, 1000000000, 0, 100000000, 10000000, 1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1}
                                        };


    BandList &blist = BandList::getBandList();
    BandInfo bi;
    bool bandOK = false;

    bool ok = false;
    QString sfreq = text();
    int sfreqLen = sfreq.length();
    int pos = cursorPosition();
    if (sfreqLen < 0 || sfreqLen > 14 || pos < 0 || pos > sfreqLen)
    {
        return;
    }
    const double tuneStep = tuningData[sfreqLen - 1][pos];


    sfreq = sfreq.trimmed().remove('.');


    double freq = sfreq.toDouble(&ok);

    if (ok)
    {
        if (direction)
        {
            freq += tuneStep;
            bandOK = blist.findBand(freq, bi);
            if (!bandOK)
            {
                freq -= tuneStep;
            }
        }
        else
        {
            freq -= tuneStep;
            bandOK = blist.findBand(freq, bi);
            if (!bandOK)
            {
                freq += tuneStep;
            }
        }


        sfreq = convertFreqToStr(freq);
        trace(QString("Change Freq: Freq Tuning = %1").arg(sfreq));
        if (bandOK)
        {
            setText(convertFreqStrDisp(sfreq));
            emit newFreq();
        }
        else
        {
            setText(QString("%1 %2 %3").arg("<font color='Red'>").arg(convertFreqStrDisp(sfreq)).arg("</font>"));
        }

        setCursorPosition(pos);
   }
}


