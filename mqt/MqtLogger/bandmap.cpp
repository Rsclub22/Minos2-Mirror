#include "bandmap.h"
#include <QDebug>


#include <QLabel>
#include <QList>
#include <QMapIterator>
#include <QVector>
#include <QVectorIterator>

// holds all the callsigns added to the bandmap


Bandmap::Bandmap()
{



}




void Bandmap::addCallsignMarker(double frequency, QString callsign, QString time, int spotType, int callBearing)
{

    QVector <CallsignMarker> markerList;
    QMap<double, QVector<CallsignMarker>>::iterator i;

    CallsignMarker newMark(callsign, time, spotType, callBearing);
    markerList << newMark;

    if (markers.empty()|| !markers.contains(frequency))
    {
        markers.insert(frequency, markerList);
    }
    else
    {
        bool callFound = false;
        QVector<CallsignMarker> s;
        // check if callsign exists
        for (i = markers.begin(); i != markers.end(); ++i)
        {

            s = i.value();
            for (int x = 0; x < s.count(); x++)
            {
                if (callsign == s[x].getcallSign())
                {
                   callFound = true;
                   if (i.key() != frequency)
                   {
                       s.removeAt(x);
                       markers.insert(frequency, s);
                   }
                }
            }
        }
        if (!callFound)
        {
//            bool freqFound = false;
            // does the frequency exist?
            for (i = markers.begin(); i != markers.end(); ++i)
            {
                if (i.key() == frequency)
                {
                    //freqFound = true;
                    s = i.value();
                    if (s.count() == mapData::MAX_CALLSIGNS)
                    {
                        s.removeFirst();
                    }
                    s.append(newMark);
                    markers.insert(frequency, s);

                }
            }
//            if (!freqFound)
//            {
//                markers.insert(frequency, markerList);
//            }
        }
    }
}



/*
void  Bandmap::addCallsignToDial(QPainter *painter)
{
    painter->setPen(Qt::blue);
    painter->setFont(QFont("Arial", 30));
    painter->drawText(0,0,"G8FKH");
}



void Bandmap::testMarker()
{



    CallsignMarker marker1("G8FKH", "18:30", 0, 200);
    CallsignMarker marker2("G8LZE", "19:35", 1, 250);
    CallsignMarker marker3("GD8EXI", "19:36", 2, 100);
    CallsignMarker marker4("GM3SEK", "19:36", 2, 100);
    CallsignMarker marker5("G3ABC", "19:40", 3, 299);
    CallsignMarker marker6("G4GHJ", "20:41", 2, 100);
    CallsignMarker marker7("M0DGB", "21:36", 4, 041);
    CallsignMarker marker8("2E0NEY", "21:34", 3, 234);
    CallsignMarker marker9("M0SAT", "22:00", 1, 050);

    QVector <CallsignMarker> markerList;





    //QMap<double, QVector<CallsignMarker>> map1;

    markerList << marker1 << marker2 << marker3;
    markers.insert(144300250, markerList);
    markerList.clear();

    markerList << marker4 << marker5 << marker6;
    markers.insert(144210567, markerList);
    markerList.clear();

    markerList << marker7 << marker8 << marker9;
    markers.insert(144176487, markerList);
    markerList.clear();



    QMap<double, QVector<CallsignMarker>>::iterator i;
    QVector<CallsignMarker> s;
    for (i = markers.begin(); i != markers.end(); ++i)
    {
        s = i.value();
        QString t = i.value()[0].getcallSign();
       qDebug() << i.key() << ": " << s[0].getcallSign() << t << endl;
    }



    int a = 0;
    a = 1;



}
*/
