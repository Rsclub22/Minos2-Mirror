


#include "rotControlFrameBase.h"

#include "LoggerContest.h"
#include "cutils.h"
#include "tlogcontainer.h"
#include "SendRPCDM.h"
#include "rotatorcommon.h"
#include "MTrace.h"






RotControlFrameBase::RotControlFrameBase(QWidget *parent):
    QFrame(parent)
    , ct(nullptr)
{




}



RotControlFrameBase::~RotControlFrameBase()
{


}

void RotControlFrameBase::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);
    setRotatorList();
}

int RotControlFrameBase::getAngle(QString brgSt)
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

int RotControlFrameBase::getCurrentBearing()
{
    return currentBearing;
}


QString RotControlFrameBase::convertBearingForDisplay(QString bearing)
{
    // bearing could be decorated with characters which are removed for display in
    // the bearing edit box.
    QString brgbuff = bearing;
    brgbuff.remove(DEGREE_SYMBOL).remove(BEARING_TRUE_CHAR).remove(SHORTLOC_DELIMITER_START).remove(SHORTLOC_DELIMITER_END).remove(SHORTLOCATOR_IDENTIFIER);


    traceMsg(QString("Convert Bearing for Display = %1").arg(brgbuff));
    return brgbuff;
}


// *************************** slot
void RotControlFrameBase::setBrgFromQSOLog(QString brg)
{
    // bearing arrives here correctly formatted for display
    if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        if (!brg.isEmpty())
        {
            traceMsg("Bearing from QSO Log " + brg);
            setTurnDisplayText(convertBearingForDisplay(brg));
        }
    }
}
// ************************* slot
// Note! This comes from the single click on Match Frame Entry. Double-click is via
// QSOLog Frame
void RotControlFrameBase::setBrgFromMatchFrame(QString brg)
{
    if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        traceMsg(QString("Set bearing from single click match frame - %1").arg(brg));
        setTurnDisplayText(convertBearingForDisplay(brg));
    }
}


// ************************* slot
// Note! The bearing string from memory could have '#' appended to denote
// bearing was calculated from a short locator.
void RotControlFrameBase::setBrgFromFrmMemory(QString brg)
{
    if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        traceMsg("Set Bearing from memory " + brg);
        setTurnDisplayText(convertBearingForDisplay(brg));
    }

}

// ************************* slot
// Note! The bearing string from cluster spot could have '#' appended to denote
// bearing was calculated from a short locator.
void RotControlFrameBase::setBrgFromSpot(QString brg)
{
    if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        traceMsg(QString("Set Bearing from spot %1").arg(brg));
        setTurnDisplayText(convertBearingForDisplay(brg));
        BearingLineEdit* le = getBrgLineEditObject();
        if (le)
        {
            traceMsg(QString("Bearing text box from spot %1").arg(le->text()));
        }

        turnTo(getAngle(brg));
    }
}


void RotControlFrameBase::setTurnDisplayText(QString brg)
{
    BearingLineEdit* brglne = getBrgLineEditObject();

    if (brglne)
    {
       brglne->setText(brg);
    }

}



void RotControlFrameBase::initConnections()
{

    connect(this, &RotControlFrameBase::bearingEditReturn, this, &RotControlFrameBase::on_Rotate_clicked);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::BrgStrToRot, this, &RotControlFrameBase::setBrgFromQSOLog);

    // from match frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MatchBrgStrToRot, this, &RotControlFrameBase::setBrgFromMatchFrame);

    // from cluster frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SpotBrgStrToRot, this, &RotControlFrameBase::setBrgFromSpot);

    // from memory frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MemBrgStrToRot, this, &RotControlFrameBase::setBrgFromFrmMemory);

}

void RotControlFrameBase::turnTo(int angle)
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
                QLabel* stateLabel = getRotConnectStateLabelObject();
                if (stateLabel)
                {
                    stateLabel->setText(msg);
                }
                return;
            }


            //if (angle > maxAzimuth)
            if (angle > COMPASS_MAX360)
            {
                traceMsg(QString("TurnTo: Bearing too large"));
                QString msg = HtmlFontColour(Qt::red) + tr("Bearing too large - %1").arg(angle);
                QLabel* stateLabel = getRotConnectStateLabelObject();
                if (stateLabel)
                {
                    stateLabel->setText(msg);
                }

                return;
            }
            //else if (angle < minAzimuth)
            else if (angle < COMPASS_MIN0)
            {
                traceMsg(QString("TurnTo: Bearing too small"));
                QString msg = HtmlFontColour(Qt::red) + tr("Bearing too small - %1").arg(angle);
                QLabel* stateLabel = getRotConnectStateLabelObject();
                if (stateLabel)
                {
                    stateLabel->setText(msg);
                }

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

void RotControlFrameBase::on_Rotate_clicked()
{
    if (rotConnected && !rotError)
    {
        traceMsg("Turn to button Clicked");
        BearingLineEdit* le = getBrgLineEditObject();
        if (le)
        {
            QString brgStr = le->text().trimmed();
            if (!brgStr.isEmpty() && le->isValid())
            {
                setTurnDisplayText(convertBearingForDisplay(brgStr));
                le->selectAll();
                turnTo(getAngle(brgStr));


            }
        }
    }
    else
    {
        traceMsg(QString("On Rotate:Rotconnected = %1, RotError = %2").arg(rotConnected).arg(rotError));
    }


}

void RotControlFrameBase::on_nudgeLeft_clicked()
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

void RotControlFrameBase::on_nudgeRight_clicked()
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

void RotControlFrameBase::on_RotateLeft_clicked()
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
            traceMsg(QString("Current Bearing = %1 <= minAzimuth %2").arg(QString::number(rotatorBearing), QString::number(minAzimuth)));
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

void RotControlFrameBase::clearBearingLineEdit()
{
    BearingLineEdit* le = getBrgLineEditObject();
    if (le)
    {
        le->clear();
    }
}


void RotControlFrameBase::on_RotateRight_clicked()
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
            traceMsg(QString("Current Bearing = %1 >= maxAzimuth %2").arg(QString::number(currentBearing), QString::number(maxAzimuth)));
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



void RotControlFrameBase::rot_left_button_on()
{
    rot_left_button_status = true;
    showRotLeftButOn();
}

void RotControlFrameBase::rot_left_button_off()
{
    rot_left_button_status = false;
    showRotLeftButOff();
}

void RotControlFrameBase::rot_right_button_on()
{
    rot_right_button_status = true;
    showRotRightButOn();
}

void RotControlFrameBase::rot_right_button_off()
{
    rot_right_button_status = false;
    showRotRightButOff();
}


void RotControlFrameBase::showTurnButOn()
{
    //ui->Rotate->setPalette(*redText);
    QToolButton* tb = getRotateButtonObject();
    if (tb)
    {
        tb->setStyleSheet(BUTTON_ON_STYLE);
        tb->setText(tr("Turn"));
    }

}

void RotControlFrameBase::showTurnButOff()
{
    QToolButton* tb = getRotateButtonObject();
    if (tb)
    {
        tb->setStyleSheet(BUTTON_OFF_STYLE);
        tb->setText(tr("Turn"));
    }
}




void RotControlFrameBase::showRotLeftButOn()
{
    QToolButton* tb = getRotateLeftObject();
    if (tb)
    {
        tb->setStyleSheet(BUTTON_ON_STYLE);
        tb->setText(tr("(CCW) Left"));
    }

}

void RotControlFrameBase::showRotLeftButOff()
{
    QToolButton* tb = getRotateLeftObject();
    if (tb)
    {
        tb->setStyleSheet(BUTTON_OFF_STYLE);
        tb->setText(tr("(CCW) Left"));
    }
}

void RotControlFrameBase::showRotRightButOn()
{
    QToolButton* tb = getRotateRightObject();
    if (tb)
    {
        tb->setStyleSheet(BUTTON_ON_STYLE);
        tb->setText(tr("(CW) Right"));
    }
}

void RotControlFrameBase::showRotRightButOff()
{
    QToolButton* tb = getRotateRightObject();
    if (tb)
    {
        tb->setStyleSheet(BUTTON_OFF_STYLE);
        tb->setText(tr("(CW) Right"));
    }
}

void RotControlFrameBase::on_StopRotate_clicked()
{
    emit sendRotator(rpcConstants::eRotateStop, 0);
    clearRotatorFlags();
    showTurnButOff();
    showRotLeftButOff();
    showRotRightButOff();
}

void RotControlFrameBase::clearRotatorFlags()
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

bool RotControlFrameBase::isRotatorLoaded()
{
    return LogContainer->sendDM->isRotatorLoaded();
}
void RotControlFrameBase::setRotatorList()
{
    QStringList rots = LogContainer->sendDM->rotators();

    QComboBox* cb = getAntennaSelectObject();
    if (cb)
    {
       comboSetUniqueNames(rots, cb);
    }


    if (ct && !ct->isReadOnly())
    {
        setRotatorAntennaName(ct->antennaName.getValue().toString());
    }
}

void RotControlFrameBase::setRotatorState(const QString &s)
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
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                   lbl->setText(tr("Connected"));
                }

                rotError = false;
                rotConnected = true;

                setRotatorAntennaName(ct->antennaName.getValue().toString()); // make sure the name appears

                emit rotatorConnected(true);     // tell bandmap
            }
            else if (lastConnectStat == ROT_STATUS_DISCONNECTED)
            {
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                    lbl->setText(tr("Disconnected"));
                }
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
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                    lbl->setText(tr("Stop"));
                }

                rotError = false;
                clearRotatorFlags();
                showRotLeftButOff();
                showRotRightButOff();
                showTurnButOff();
            }
            else if (lastStatus == ROT_STATUS_ROTATE_CCW)
            {
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                   lbl->setText(HtmlFontColour("Green") + tr("Rotating CCW"));
                }

                rotError = false;
                moving = false;
                movingCW = false;
                movingCCW = true;
                // clearRotatorFlags();
                showRotLeftButOn();
            }
            else if (lastStatus == ROT_STATUS_ROTATE_CW)
            {
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                    lbl->setText(HtmlFontColour("Green") + tr("Rotating CW"));
                }

                rotError = false;
                moving = false;
                movingCW = true;
                movingCCW = false;
                //clearRotatorFlags();
                showRotRightButOn();
            }
            else if (lastStatus == ROT_STATUS_TURN_TO)
            {

                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                    lbl->setText(HtmlFontColour("Green") + tr("Turning to bearing"));
                }

                rotError = false;
                moving = true;
                movingCW = false;
                movingCCW = false;
                showTurnButOn();
                //clearRotatorFlags();
            }
            else if (lastStatus == ROT_STATUS_CONNECTED)
            {
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                    lbl->setText(tr("Connected"));
                }

                rotError = false;
                rotConnected = true;
            }
            else if (lastStatus == ROT_STATUS_DISCONNECTED)
            {
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                    lbl->setText(tr("Disconnected"));
                }

                rotError = false;
                rotConnected = false;
            }
            else if (lastStatus == ROT_STATUS_ERROR)
            {
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                    lbl->setText(HtmlFontColour("Red") + tr("Error"));
                }

                rotError = true;
            }
            else
            {
                QLabel* lbl = getRotConnectStateLabelObject();
                if (lbl)
                {
                    lbl->setText(lastStatus);
                }
            }

        }
    }
    if (sl.count() <= 1)     // will be a revoked state
    {
        QLabel* lbl = getRotConnectStateLabelObject();
        if (lbl)
        {
            lbl->setText(tr("Disconnected"));
        }

        rotError = false;
        rotConnected = false;
    }



}

void RotControlFrameBase::setRotatorAntennaName(const QString &s)
{
    traceMsg("Set Antenna Name = " + s);
    QComboBox* cb = getAntennaSelectObject();
    int index = cb->findData(s);
    if (index >= 0)
        cb->setCurrentIndex(index);
    else
        trace(QString("Antenna %1 not found").arg(s));

    antennaName = cb->currentData().toString();
    if (ct && !ct->isReadOnly())
    {
        emit selectRotator(antennaName);
    }
}
void RotControlFrameBase::on_ContestPageChanged()
{
    // send rotator select to rotator app

    if (ct)
        emit selectRotator(ct->antennaName.getValue().toString());
}

void RotControlFrameBase::setRotatorBearing(const QString &s)
{
    traceMsg(QString("Bearings from rotator control = %1").arg(s));

    QLabel* lbl = getRotBrgDisplayObject();
    if (lbl)
    {
        displayCompassBearingWithOverlap(s, rotatorBearing, currentBearing, lbl);
    }



}

void RotControlFrameBase::setRotatorMaxAzimuth(const int maxAz)
{
    traceMsg(QString("Set MaxAzimuth = %1").arg(QString::number(maxAz)));

    maxAzimuth = maxAz;

}


void RotControlFrameBase::setRotatorMinAzimuth(const int minAz)
{
    traceMsg(QString("Set MinAzimuth = %1").arg(QString::number(minAz)));

    minAzimuth = minAz;

}

void RotControlFrameBase::setSupportStopCommandFlag(bool state)
{
    traceMsg((QString("Set Support Stop Command Flag = %1").arg(state ?  "True" : "False" )));
    supportStopCommand = state;
    QToolButton* tb = getStopRotateObject();
    if (tb)
    {
        tb->setVisible(state);
    }

}


void RotControlFrameBase::setCwCcwCmdEnable(bool s)
{
    supportCwCcwCmd = s;
    setCwCcW_Items_Visible(s);
}

void RotControlFrameBase::setCwCcW_Items_Visible(bool visible)
{
    QToolButton* tb = getnudgeLeftObject();
    if (tb)
    {
       tb->setVisible(visible);
    }

    tb =getnudgeRightObject();
    if (tb)
    {
        tb->setVisible(visible);
    }

    tb = getRotateLeftObject();
    if (tb)
    {
        tb->setVisible(visible);
    }

    tb = getRotateRightObject();
    if (tb)
    {
        tb->setVisible(visible);
    }

}

void RotControlFrameBase::skyScanStartedSetFrameDisabled(bool disabled)
{
    traceMsg(QString("skyScan set frame disabable = %1").arg(disabled ? "True" : "False"));

    QGroupBox* gb = getRotatorControlGroupBoxObject();
    if (gb)
    {
        gb->setDisabled(disabled);
    }


}

void RotControlFrameBase::traceMsg(QString msg)
{
    trace(QString("[%1] %2 - %3").arg(frameName, antennaName, msg));
}

void RotControlFrameBase::on_antennaNameSel_activated(int /*arg1*/)
{
    QComboBox* cb = getAntennaSelectObject();
    if (cb)
    {
        antennaName = cb->currentData().toString();
    }


    emit selectRotator(antennaName);

}
void RotControlFrameBase::getRotDetails(memoryData::memData &m)
{
    m.bearing = currentBearing;
}

void RotControlFrameBase::presetTurn(QString b)
{
    if (!b.isEmpty())
    {
        turnTo(b.toInt());
        //ui->BrgSt->setText(b);
        setTurnDisplayText(convertBearingForDisplay(b));
        QLineEdit* le = getBrgLineEditObject();
        if (le)
        {
           le->setFocus();
        }

    }
}

void RotControlFrameBase::keyPressEvent(QKeyEvent *event)
{

    int Key = event->key();

    /*
    Qt::KeyboardModifiers mods = event->modifiers();
    bool shift = mods & Qt::ShiftModifier;
    bool ctrl = mods & Qt::ControlModifier;
    bool alt = mods & Qt::AltModifier;
*/

    BearingLineEdit* le = getBrgLineEditObject();
    bool test = false;
    if (le)
    {
        if (Key == Qt::Key_Return && le->hasFocus())
        {
            test = true;
        }
    }

    if (test)
    {
        emit bearingEditReturn();
    }
    else
    {
        QFrame::keyPressEvent(event);
    }

}



void RotControlFrameBase::checkConnection()
{
    QString loggerUuid = LogContainer->sendDM->getLoggerUuid();
    PubSubName rotSelected = LogContainer->sendDM->getSelectedRot(loggerUuid);

    if (rotSelected.isEmpty() || !isRotatorLoaded())
    {
        // clear the rot selection

        QComboBox* cb = getAntennaSelectObject();
        if (cb)
        {
           cb->setCurrentText("");
        }

        setRotatorState(ROT_STATUS_DISCONNECTED);
    }
}
