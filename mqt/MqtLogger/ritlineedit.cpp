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
#include "ritlineedit.h"



RitLineEdit::RitLineEdit(QWidget *parent):
    QLineEdit(parent)
{

    setContextMenuPolicy(Qt::NoContextMenu);
}


RitLineEdit::~RitLineEdit()
{


}





void RitLineEdit::setRitOnFlag(bool state)
{
    ritOn = state;
}



void RitLineEdit::setTensKhz(bool tensKhz)
{
    // if true support +/- 99Khz
    // false support +/- 9khz
    ritTenKHz = tensKhz;
}

void RitLineEdit::setMaxRit(int maxRit_)
{
    maxRit = maxRit_;
}

void RitLineEdit::setMinRit(int minRit_)
{
    minRit = minRit_;
}


void RitLineEdit::changeFreq(bool direction)
{
    if (ritOn)
    {

        static const int tuningData1[] = {0, 1000, 0, 100, 10};  // 0 is either +/- position or . position in display
        static const int tuningData2[] = {0, 10000, 1000, 0, 100, 10};  // 0 is either +/- position or . position in display

        bool ok = false;
        QString sfreq = text();
        int sfreqLen = sfreq.length();
        int pos = cursorPosition();
        if (pos <= 0 || pos >= sfreqLen)
        {
            return;
        }

        int tuneStep;

        if (ritTenKHz)
        {
            tuneStep = tuningData2[pos];    // support +/- 99KHz
        }
        else
        {
            tuneStep = tuningData1[pos];    // support +/- 9KHz
        }


        sfreq = sfreq.trimmed().remove('.');


        int freq = sfreq.toInt(&ok) * 10;

        if (ok)
        {
            if (direction)
            {
                freq += tuneStep;
                if (freq >= maxRit)
                {
                    freq -= tuneStep;
                }
            }
            else
            {
                freq -= tuneStep;
                if (freq <= minRit)
                {
                    freq += tuneStep;
                }
            }

            // display rit freq
            sfreq = convertRitFreqToStr(freq, ritTenKHz);
            trace(QString("Change Rit Freq: Rit Tuning = %1").arg(sfreq));
            setText(sfreq);

            // send to radio

            emit newFreq(ShortFreq(freq));

            setCursorPosition(pos);
       }



    }


}






void RitLineEdit::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->angleDelta().y() / 8;
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


void RitLineEdit::keyPressEvent(QKeyEvent *event)
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



