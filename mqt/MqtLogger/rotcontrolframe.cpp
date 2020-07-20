/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include "ContestApp.h"
#include "LoggerContest.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "qsologframe.h"
#include "SendRPCDM.h"
#include "rotatorcommon.h"
#include "rotcontrolframe.h"
#include "ui_rotcontrolframe.h"


RotControlFrame::RotControlFrame(QWidget *parent):
    QFrame(parent)
    , ct(nullptr)
    , ui(new Ui::RotControlFrame)
    , rotatorLoaded(false)
{
    ui->setupUi(this);

    ui->BrgSt->clear();

    nudgeRight1 = new QShortcut(QKeySequence("Ctrl++"), parent);   // Ctrl +
    connect(nudgeRight1, SIGNAL(activated()), ui->nudgeRight, SLOT(click()));

    nudgeRight2 = new QShortcut(QKeySequence("Ctrl+="), parent);   // Ctrl +
    connect(nudgeRight2, SIGNAL(activated()), ui->nudgeRight, SLOT(click()));

    nudgeLeft = new QShortcut(QKeySequence::ZoomOut, parent);   // Ctrl -
    connect(nudgeLeft, SIGNAL(activated()), ui->nudgeLeft, SLOT(click()));

    rotateLeft = new QShortcut(QKeySequence(ROTATE_CCW_KEY), parent);
    connect(rotateLeft, SIGNAL(activated()), this, SLOT(on_RotateLeft_clicked()));


    rotateRight = new QShortcut(QKeySequence(ROTATE_CW_KEY), parent);
    connect(rotateRight, SIGNAL(activated()), this, SLOT(on_RotateRight_clicked()));

    turnToBearing = new QShortcut(QKeySequence(ROTATE_TURN_KEY), parent);
    connect(turnToBearing, SIGNAL(activated()), this, SLOT(on_Rotate_clicked()));

    rotateStop = new QShortcut(QKeySequence(ROTATE_STOP_KEY), parent);
    connect(rotateStop, SIGNAL(activated()), this, SLOT(on_StopRotate_clicked()));


    connect(this, SIGNAL(bearingEditReturn()), this, SLOT(on_Rotate_clicked()));
    //connect(ui->BrgSt, SIGNAL(textChanged(const QString)), this, SLOT(on_BearingStTextChange(const QString)));

    connect(&MinosLoggerEvents::mle, SIGNAL(BrgStrToRot(QString)), this, SLOT(getBrgFrmQSOLog(QString)));

    // from match frame
    connect(&MinosLoggerEvents::mle, SIGNAL(MatchBrgStrToRot(QString)), this, SLOT(setBrgFromMatchFrame(QString)));

    // from cluster frame
    connect(&MinosLoggerEvents::mle, SIGNAL(SpotBrgStrToRot(QString)), this, SLOT(setBrgFromSpot(QString)));

    // from memory frame
    connect(&MinosLoggerEvents::mle, SIGNAL(MemBrgStrToRot(QString)), this, SLOT(setBrgFromFrmMemory(QString)));
    rot_left_button_off();
    rot_right_button_off();
    showTurnButOff();
    setCwCcW_Items_Visible(supportCwCcwCmd);   // init visible
    traceMsg("RotControlFrame Started");

}



RotControlFrame::~RotControlFrame()
{
    delete ui;

}

void RotControlFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);
}

int RotControlFrame::getAngle(QString brgSt)
{
    int brg = COMPASS_ERROR;

    for (int i = 0;i < brgSt.length(); i++)
    {
        if (brgSt[i].isDigit())
        {
            int j = 0;
            for (j = i; j < brgSt.length(); j++)
            {
                if (!brgSt[j].isDigit())
                {
                    break;
                }
            }
            brgSt = brgSt.mid(i, j - i);
            brg = brgSt.toInt();
            return brg;
        }
    }

    return brg;
}

int RotControlFrame::getCurrentBearing()
{
    return currentBearing;
}


QString RotControlFrame::convertBearingForDisplay(QString bearing)
{
    // bearing could be decorated with characters which are removed for display in
    // the bearing edit box.
    QString brgbuff = bearing;
    brgbuff.remove(DEGREE_SYMBOL).remove(BEARING_TRUE_CHAR).remove(SHORTLOC_DELIMITER_START).remove(SHORTLOC_DELIMITER_END).remove(SHORTLOCATOR_IDENTIFIER);

/*
    const QChar degreeChar(DEGREE_SYMBOL);
    const QChar trueChar(BEARING_TRUE_CHAR);
    const QChar shortLocDelimiterStart(SHORTLOC_DELIMITER_START);
    const QChar shortLocDelimiterEnd(SHORTLOC_DELIMITER_END);
    if (bearing.contains(SHORTLOCATOR_IDENTIFIER))
    {
        brgbuff = QString("%1%2%3%4%5").arg(shortLocDelimiterStart).arg( bearing.remove(SHORTLOCATOR_IDENTIFIER) ).arg(degreeChar).arg(trueChar).arg(shortLocDelimiterEnd);
    }
    else if (!bearing.contains(DEGREE_SYMBOL) && !bearing.contains(BEARING_TRUE_CHAR))
    {
        brgbuff = QString("%1%2%3").arg( bearing ).arg(degreeChar).arg(trueChar);
    }
    else
    {
        brgbuff = bearing;
    }
*/
    traceMsg(QString("Convert Bearing for Display = %1").arg(brgbuff));
    return brgbuff;
}



void RotControlFrame::getBrgFrmQSOLog(QString brg)
{
    // bearing arrives here correctly formatted for display
    if (!brg.isEmpty())
    {
        traceMsg("Bearing from QSO Log" + brg);
        setTurnDisplayText(convertBearingForDisplay(brg));
    }

}

// Note! This comes from the single click on Match Frame Entry. Double-click is via
// QSOLog Frame
void RotControlFrame::setBrgFromMatchFrame(QString brg)
{
    traceMsg(QString("Set bearing from single click match frame - %1").arg(brg));
     setTurnDisplayText(convertBearingForDisplay(brg));
}

/*
QString RotControlFrame::getBrgTxtFrmFrame()
{
    QString brg = ui->BrgSt->text();
    traceMsg(QString("Bearing from Bearing Edit Box = %1").arg(brg));
    return brg;
}
*/

// Note! The bearing string from memory could have '#' appended to denote
// bearing was calculated from a short locator.
void RotControlFrame::setBrgFromFrmMemory(QString brg)
{
    traceMsg("Set Bearing from memory " + brg);
    setTurnDisplayText(convertBearingForDisplay(brg));

}

// Note! The bearing string from cluster spot could have '#' appended to denote
// bearing was calculated from a short locator.
void RotControlFrame::setBrgFromSpot(QString brg)
{
    traceMsg(QString("Set Bearing from spot %1").arg(brg));
    setTurnDisplayText(convertBearingForDisplay(brg));
    traceMsg(QString("Bearing text box from spot %1").arg(ui->BrgSt->text()));
    turnTo(getAngle(brg));
}


void RotControlFrame::setTurnDisplayText(QString brg)
{
    ui->BrgSt->setText(brg);
}


void RotControlFrame::turnTo(int angle)
{
    traceMsg("Turn to - " + QString::number(angle));

    if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        //ui->BrgSt->setText(bearingForDisplay(angle));

        if (rotConnected)
        {
            traceMsg("Turn to - Contest match and connected");

            if (angle == COMPASS_ERROR)
            {
                traceMsg(QString("TurnTo: Bearing empty or invalid"));
                QString msg = HtmlFontColour(Qt::red) + tr("Bearing empty or invalid");
                ui->rotatorStatMsg->setText(msg);
                return;
            }


            //if (angle > maxAzimuth)
            if (angle > COMPASS_MAX360)
            {
                traceMsg(QString("TurnTo: Bearing too large"));
                QString msg = HtmlFontColour(Qt::red) + tr("Bearing too large - %1").arg(angle);
                ui->rotatorStatMsg->setText(msg);
                return;
            }
            //else if (angle < minAzimuth)
            else if (angle < COMPASS_MIN0)
            {
                traceMsg(QString("TurnTo: Bearing too small"));
                QString msg = HtmlFontColour(Qt::red) + tr("Bearing too small - %1").arg(angle);
                ui->rotatorStatMsg->setText(msg);
                return;
            }
            else if (angle == currentBearing)
            {
                traceMsg(QString("TurnTo: Bearing = CurrentBearing"));
                return;
            }
            else
            {
                traceMsg(QString("Send Bearing %1 to Rotator Control").arg(QString::number(angle)));
                emit sendRotator(rpcConstants::eRotateDirect, angle);
                showTurnButOn();
                moving = true;
            }


        }

    }
}

void RotControlFrame::on_Rotate_clicked()
{
    if (rotConnected && !rotError)
    {
        traceMsg("Turn to button Clicked");
        QString brgStr = ui->BrgSt->text().trimmed();
        if (!brgStr.isEmpty() && ui->BrgSt->isValid())
        {
            setTurnDisplayText(convertBearingForDisplay(brgStr));
            ui->BrgSt->selectAll();
            turnTo(getAngle(brgStr));


        }
    }
    else
    {
        traceMsg(QString("On Rotate:Rotconnected = %1, RotError = %2").arg(rotConnected).arg(rotError));
    }


}

void RotControlFrame::on_nudgeLeft_clicked()
{
    if (rotConnected && !rotError)
    {
        traceMsg("Nudge Left Clicked");

        int newBearing = currentBearing - 3;
        if (newBearing < 0)
            newBearing += 360;
        setTurnDisplayText(convertBearingForDisplay(QString::number(newBearing)));
        turnTo(newBearing);
    }
    else
    {
        traceMsg(QString("NudgeLeft:Rotconnected = %1, RotError = %2").arg(rotConnected).arg(rotError));
    }
}

void RotControlFrame::on_nudgeRight_clicked()
{

    if (rotConnected && !rotError)
    {
        traceMsg("Nudge Right Clicked");
        int newBearing = currentBearing + 3;
        if (newBearing >= 360)
            newBearing -= 360;
        setTurnDisplayText(convertBearingForDisplay(QString::number(newBearing)));
        turnTo(newBearing);
    }
    else
    {
        traceMsg(QString("NudgeRight:Rotconnected = %1, RotError = %2").arg(rotConnected).arg(rotError));
    }

}

void RotControlFrame::on_RotateLeft_clicked()
{

    if (!rotConnected || rotError)
    {
        traceMsg(QString("On Rotate Left:Rotconnected = %1, RotError = %2").arg(rotConnected).arg(rotError));
        return;
    }

    traceMsg("RotLeft Button Clicked");
    if (rot_left_button_status)
    {
        traceMsg("RotLeft Button On - Stop and Turn Off");
        on_StopRotate_clicked();
        rot_left_button_off();

    }
    else
    {
        traceMsg("Current Bearing = " + QString::number(currentBearing));
        traceMsg("Rotator Bearing = " + QString::number(rotatorBearing));
        traceMsg("RotLeft Status = " + QString::number(rot_left_button_status));
        int angle = 0;

        if (rotatorBearing <= minAzimuth)
        {
            traceMsg(QString("Current Bearing = %1 <= minAzimuth %2").arg(QString::number(rotatorBearing)).arg( QString::number(minAzimuth)));
            return;
        }

        if (moving || movingCW || movingCCW)
        {
            traceMsg("RotLeft Stopping");
            on_StopRotate_clicked();
        }

        rot_left_button_on();
        traceMsg("Send RotLeft to Rototor Control");
        emit sendRotator(rpcConstants::eRotateLeft, angle);
        movingCW = true;
        }

}


void RotControlFrame::on_RotateRight_clicked()
{

    if (!rotConnected || rotError)
    {
        traceMsg(QString("On Rotate Right:Rotconnected = %1, RotError = %2").arg(rotConnected).arg(rotError));
        return;
    }


    traceMsg("RotRight Button Clicked");

    if (rot_right_button_status)
    {
        traceMsg("RotRight Button On - Stop and Turn Off");
        on_StopRotate_clicked();
        rot_right_button_off();

    }
    else
    {
        traceMsg("Current Bearing = " + QString::number(currentBearing));
        traceMsg("Rotator Bearing = " + QString::number(rotatorBearing));
        traceMsg("RotRight Status = " + QString::number(rot_right_button_status));
        int angle = 0;

        if (rotatorBearing >= maxAzimuth)
        {
            traceMsg(QString("Current Bearing = %1 >= maxAzimuth %2").arg(QString::number(currentBearing)).arg(QString::number(maxAzimuth)));
            return;
        }

        if (moving || movingCW || movingCCW)
        {
            traceMsg("RotRight Stopping");
            on_StopRotate_clicked();
        }

        rot_right_button_on();
        traceMsg("Send RotRight to Rotator Control");
        emit sendRotator(rpcConstants::eRotateRight, angle);
        movingCCW = true;
        }

}

/*
void RotControlFrame::on_BearingStTextChange(const QString brg)
{

    if (validateBearingEntry(brg) || brg.isEmpty())
    {
        // set frame to black
        ui->BrgSt->setStyleSheet("QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: black ; color : black}");
    }
    else
    {
        // set frame to red
        ui->BrgSt->setStyleSheet("QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: red ; color : black}");
    }
}


bool RotControlFrame::validateBearingEntry(const QString brg)
{
    QString bearing = brg;

    bearing = bearing.trimmed().remove(DEGREE_SYMBOL, Qt::CaseInsensitive).remove(BEARING_TRUE_CHAR).remove(SHORTLOC_DELIMITER_START).remove(SHORTLOC_DELIMITER_END);
    bool ok;
    int br = bearing.toInt(&ok);
    if ((br >= COMPASS_MIN0 && br <= COMPASS_MAX360 && ok) )
    {
       return true;
    }
    else
    {
        return false;
    }
}

*/

void RotControlFrame::keyPressEvent(QKeyEvent *event)
{

    int Key = event->key();

/*
    Qt::KeyboardModifiers mods = event->modifiers();
    bool shift = mods & Qt::ShiftModifier;
    bool ctrl = mods & Qt::ControlModifier;
    bool alt = mods & Qt::AltModifier;
*/

    if (Key == Qt::Key_Return && ui->BrgSt->hasFocus())
    {
        emit bearingEditReturn();
    }
    else
    {
        QFrame::keyPressEvent(event);
    }

}




void RotControlFrame::rot_left_button_on()
{
    rot_left_button_status = true;
    showRotLeftButOn();
}

void RotControlFrame::rot_left_button_off()
{
    rot_left_button_status = false;
    showRotLeftButOff();
}

void RotControlFrame::rot_right_button_on()
{
    rot_right_button_status = true;
    showRotRightButOn();
}

void RotControlFrame::rot_right_button_off()
{
    rot_right_button_status = false;
    showRotRightButOff();
}


void RotControlFrame::showTurnButOn()
{
    //ui->Rotate->setPalette(*redText);
    ui->Rotate->setStyleSheet(BUTTON_ON_STYLE);
    ui->Rotate->setText(tr("Turn"));
}

void RotControlFrame::showTurnButOff()
{
    //ui->Rotate->setPalette(*blackText);
    ui->Rotate->setStyleSheet(BUTTON_OFF_STYLE);
    ui->Rotate->setText(tr("Turn"));
}




void RotControlFrame::showRotLeftButOn()
{
    //ui->RotateLeft->setPalette(*redText);
    ui->RotateLeft->setStyleSheet(BUTTON_ON_STYLE);
    ui->RotateLeft->setText(tr("(CCW) Left"));
}

void RotControlFrame::showRotLeftButOff()
{
    //ui->RotateLeft->setPalette(*blackText);
    ui->RotateLeft->setStyleSheet(BUTTON_OFF_STYLE);
    ui->RotateLeft->setText(tr("(CCW) Left"));
}

void RotControlFrame::showRotRightButOn()
{
    //ui->RotateRight->setPalette(*redText);
    ui->RotateRight->setStyleSheet(BUTTON_ON_STYLE);
    ui->RotateRight->setText(tr("(CW) Right"));
}

void RotControlFrame::showRotRightButOff()
{
    //ui->RotateRight->setPalette(*blackText);
    ui->RotateRight->setStyleSheet(BUTTON_OFF_STYLE);
    ui->RotateRight->setText(tr("(CW) Right"));
}

void RotControlFrame::on_StopRotate_clicked()
{
    emit sendRotator(rpcConstants::eRotateStop, 0);
    clearRotatorFlags();
    showTurnButOff();
    showRotLeftButOff();
    showRotRightButOff();
}

void RotControlFrame::clearRotatorFlags()
{
    rot_left_button_off();
    rot_right_button_off();
    moving = false;
    movingCCW = false;
    movingCW = false;
    showTurnButOff();
    showRotLeftButOff();
    showRotRightButOff();
}

void RotControlFrame::setRotatorLoaded()
{
    rotatorLoaded = true;
}
bool RotControlFrame::isRotatorLoaded()
{
    return rotatorLoaded;
}
void RotControlFrame::setRotatorList()
{
    QStringList rots = LogContainer->sendDM->rotators();

    ui->antennaName->clear();
    ui->antennaName->addItem("");
    ui->antennaName->addItems(rots);

    if (ct && !ct->isProtected())
    {
        setRotatorAntennaName(ct->antennaName.getValue().toString());
    }
}

void RotControlFrame::setRotatorState(const QString &s)
{


       traceMsg("Set Rotator State = " + s);
       // split the message
       QStringList sl = s.split(':');

       if (sl.count() < 3)
       {
           if (sl.count() > 0 && sl[0] != lastConnectStat)
           {
               lastConnectStat = sl[0];
               if (lastConnectStat == ROT_STATUS_CONNECTED)
               {
                   ui->rotConnectState->setText(tr("Connected"));
                   rotError = false;
                   rotConnected = true;
                   setRotatorAntennaName(ct->antennaName.getValue().toString()); // make sure the name appears

                   emit rotatorConnected(true);     // tell bandmap
               }
               else if (lastConnectStat == ROT_STATUS_DISCONNECTED)
               {
                   ui->rotConnectState->setText(tr("Disconnected"));
                   rotError = false;
                   rotConnected = false;
                   emit rotatorConnected(false);     // tell bandmap


               }
           }
           if (sl.count() > 1 && sl[1] != lastStatus)
           {
               lastStatus = sl[1];

               if (lastStatus == ROT_STATUS_STOP)
               {
                   ui->rotatorStatMsg->setText(tr("Stop"));
                   rotError = false;
                   clearRotatorFlags();
                   showRotLeftButOff();
                   showRotRightButOff();
                   showTurnButOff();
               }
               else if (lastStatus == ROT_STATUS_ROTATE_CCW)
               {
                   ui->rotatorStatMsg->setText(HtmlFontColour("Green") + tr("Rotating CCW"));
                   rotError = false;
                   moving = false;
                   movingCW = false;
                   movingCCW = true;
                  // clearRotatorFlags();
                   showRotLeftButOn();
               }
               else if (lastStatus == ROT_STATUS_ROTATE_CW)
               {
                   ui->rotatorStatMsg->setText(HtmlFontColour("Green") + tr("Rotating CW"));
                   rotError = false;
                   moving = false;
                   movingCW = true;
                   movingCCW = false;
                   //clearRotatorFlags();
                   showRotRightButOn();
               }
               else if (lastStatus == ROT_STATUS_TURN_TO)
               {

                       ui->rotatorStatMsg->setText(HtmlFontColour("Green") + tr("Turning to bearing"));
                       rotError = false;
                       moving = true;
                       movingCW = false;
                       movingCCW = false;
                       showTurnButOn();
                       //clearRotatorFlags();
               }
               else if (lastStatus == ROT_STATUS_CONNECTED)
               {
                   ui->rotConnectState->setText(tr("Connected"));
                   rotError = false;
                   rotConnected = true;
               }
               else if (lastStatus == ROT_STATUS_DISCONNECTED)
               {
                   ui->rotConnectState->setText(tr("Disconnected"));
                   rotError = false;
                   rotConnected = false;
               }
               else if (lastStatus == ROT_STATUS_ERROR)
               {
                   ui->rotatorStatMsg->setText(HtmlFontColour("Red") + tr("Error"));
                   rotError = true;
               }
               else
               {
                   ui->rotatorStatMsg->setText(lastStatus);
               }

           }
       }
       if (sl.count() <= 1)     // will be a revoked state
       {
           ui->rotConnectState->setText(tr("Disconnected"));
           rotError = false;
           rotConnected = false;
       }



}

void RotControlFrame::setRotatorAntennaName(const QString &s)
{
   traceMsg("Set Antenna Name = " + s);
   int index = ui->antennaName->findText(s, Qt::MatchFixedString);
   if (index >= 0)
       ui->antennaName->setCurrentIndex(index);
   else
       ui->antennaName->setCurrentText(s);

   antennaName = ui->antennaName->currentText();
   if (ct && !ct->isProtected())
   {
        emit selectRotator(s);
   }
}
void RotControlFrame::on_ContestPageChanged()
{
    // send rotator select to rotator app

    if (ct)
        emit selectRotator(ct->antennaName.getValue().toString());
}

void RotControlFrame::setRotatorBearing(const QString &s)
{
    traceMsg("Bearings from rotator control");
    // extract displayBearing:rotatorBearing:overlapstatus
    QStringList sl = s.split(':');
    if (sl.size() < 3)
        return;

    traceMsg("Display Bearing = " + sl[0]);
    traceMsg("Rotator Bearing = " + sl[1]);
    traceMsg("OverlapStatus = " + sl[2]);

    // save rotatorBearing
    bool ok;
    rotatorBearing = sl[1].toInt(&ok, 10);

    if (!ok)
    {
        trace("Error converting rotatorBearing to int");
        return;
    }

    int iBearing = sl[0].toInt(&ok, 10);
    currentBearing = iBearing;

    if (!ok)
    {
        trace("Error converting displayBearing to int");
        return;
    }


    //QString bearing = bearing.number(iBearing);
    QString brg;
    QChar degsym = QChar(DEGREE_SYMBOL);
    //int len = bearing.length();
    int len = sl[0].length();
    if (len < 2)
    {
        brg = QString("%1%2%3")
        .arg("00").arg(sl[0]).arg(degsym);
    }
    else if (len < 3)
    {
        brg = QString("%1%2%3")
        .arg("0").arg(sl[0]).arg(degsym);
    }
    else
    {
        brg = QString("%1%2")
        .arg(sl[0]).arg(degsym);
    }

    brg.append("</font>");

    if (rotatorBearing > COMPASS_MAX360 && sl[2] == "1")
    {
        brg.prepend("<font color='Red'>");
        ui->RotBrg->setText(brg);
    }
    else if (rotatorBearing < COMPASS_MIN0 && sl[2] == "1")
    {
        brg.prepend("<font color='Blue'>");
        ui->RotBrg->setText(brg);
    }
    else
    {
        brg.prepend("<font color='Black'>");
        ui->RotBrg->setText(brg);
    }

}

void RotControlFrame::setRotatorMaxAzimuth(const int maxAz)
{
    traceMsg(QString("Set MaxAzimuth = %1").arg(QString::number(maxAz)));

    maxAzimuth = maxAz;

}


void RotControlFrame::setRotatorMinAzimuth(const int minAz)
{
    traceMsg(QString("Set MinAzimuth = %1").arg(QString::number(minAz)));

    minAzimuth = minAz;

}

void RotControlFrame::setSupportStopCommandFlag(bool state)
{
    traceMsg((QString("Set Support Stop Command Flag = %1").arg(state ?  "True" : "False" )));
    supportStopCommand = state;
}


void RotControlFrame::setCwCcwCmdEnable(bool s)
{
    supportCwCcwCmd = s;
    setCwCcW_Items_Visible(s);
}

void RotControlFrame::setCwCcW_Items_Visible(bool visible)
{
    ui->nudgeLeft->setVisible(visible);
    ui->nudgeRight->setVisible(visible);
    ui->RotateLeft->setVisible(visible);
    ui->RotateRight->setVisible(visible);
}

void RotControlFrame::traceMsg(QString msg)
{
    trace(QString("RotcontrolFrame: %1 - %2").arg(antennaName).arg( msg));
}

void RotControlFrame::on_antennaName_activated(const QString &arg1)
{

    antennaName = arg1;

    emit selectRotator(antennaName);

}
void RotControlFrame::getRotDetails(memoryData::memData &m)
{
    m.bearing = currentBearing;
}

void RotControlFrame::presetTurn(QString b)
{
    turnTo(b.toInt());
    //ui->BrgSt->setText(b);
    setTurnDisplayText(convertBearingForDisplay(b));
    ui->BrgSt->setFocus();
}
void RotControlFrame::checkConnection()
{
    QString loggerUuid = LogContainer->sendDM->getLoggerUuid();
    PubSubName rotSelected = LogContainer->sendDM->getSelectedRot(loggerUuid);

    if (rotSelected.isEmpty())
    {
        // clear the rot selection

        ui->antennaName->setCurrentText("");
        setRotatorState(ROT_STATUS_DISCONNECTED);
    }
}

void RotControlFrame::closeContest()
{
    // is this the last frame
//    if (LogContainer->getLogFrameCount() == 1 && rotConnected)
//    {
//        trace(QString("Last contest frame closing, close rotator connection"));
//        setRotatorAntennaName("");
//    }
}
