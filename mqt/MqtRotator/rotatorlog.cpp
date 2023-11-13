/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QtDebug>
#include <QDateTime>
#include <QSettings>

#include "fileutils.h"
#include "rotatorcommon.h"
#include "rotatorlog.h"

RotatorLog::RotatorLog()
{
    oldBearing = "";
    firstBearing = false;
    moving = false;
}

void RotatorLog::saveBearingLog(int bearing)
{
    static int oldbearing = COMPASS_ERROR;

    if (oldbearing != bearing && bearingLogEnabled)
    {
        if (oldbearing > bearing)
        {
            if ((oldbearing - bearing) > bearingLogBearingDiff)
            {
                oldbearing = bearing;
                writeLog(bearing);
            }
        }
        else
        {
            if ((bearing - oldbearing) > bearingLogBearingDiff)
            {
                oldbearing = bearing;
                writeLog(bearing);
            }
        }
    }


}



int RotatorLog::writeLog(int bearing)
{
    int compassBearing = bearing;
    if (compassBearing > COMPASS_MAX360)
    {
        compassBearing -= COMPASS_MAX360;
    }
    QString fileName = bearingLogDir;
    if (!QDir(fileName).exists())
    {
        QDir().mkdir(fileName);
    }


    if (bearingLogEnabled)
    {

       fileName.append('/');
       fileName.append(bearingLogFileName);
       fileName.append(BEARINGLOG_FILETYPE);

       QString sbearing = QString::number(compassBearing);


        if (oldBearing != sbearing && !firstBearing && !moving)
        {
            oldBearing = sbearing;
            writeBearing(sbearing, fileName);
            firstBearing = true;
            moving = true;
            return 0;
        }



        if (moving && firstBearing)
        {
            if (oldBearing != sbearing)
            {
                oldBearing = sbearing;
                writeBearing(sbearing, fileName);
                return 0;
            }


            if (oldBearing == sbearing)
            {
                moving = false;
                firstBearing = false;
                writeBearing(sbearing, fileName);
                return 0;
            }
        }
     }

    return 0;
}



int RotatorLog::writeBearing(const QString bearing, QString filename)
{
        QDateTime t = QDateTime::currentDateTime();
        QString t_str = t.toString();

        QFile file( filename );
        if( !file.exists() )
        {
            //logMessage("The file" + file.fileName() + "does not exist.");
            //return -1;
        }
        if( !file.open( QIODevice::Append | QIODevice::Text) )
        {
            //logMessage("Could not open" + file.fileName() + "for writing.");
            //return -1;
        }


        QTextStream stream(&file);

        stream << t_str << " Bearing is " + bearing + " Degrees" + "\n";

        file.close();
        return 0;
}


void RotatorLog::getBearingLogConfig()
{
    QSettings config(getDirectoryLocation(dlConfiguration) + "/MinosRotatorConfig.ini", QSettings::IniFormat);
    config.beginGroup("BearingLog");

    bearingLogDir = config.value("directory", "./BearingLog").toString();
    bearingLogFileName = config.value("filename", "bearing").toString();
    bearingLogBearingDiff = config.value("bearing_difference", 0).toInt();
    bearingLogEnabled = config.value("loggingEnabled", false).toBool();

    config.endGroup();
}




