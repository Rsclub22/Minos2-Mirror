/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
//                      Rotator Control Frame Base for Logger
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////



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
    if (ct)
    {
        setRotatorList();
    }
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
    return rotFrameData.getAntennaBearing();
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


void RotControlFrameBase::setupShortcuts(QWidget *shortcutParent)
{
    if (!shortcutParent)
        return;

    // Nudge Right - Ctrl++
    nudgeRight1 = new QShortcut(QKeySequence("Ctrl++"), shortcutParent);
    if (auto btn = getnudgeRightObject())
    {
        connect(nudgeRight1, &QShortcut::activated, btn, &QToolButton::click);
    }


    // Nudge Right - Ctrl+= (also "+")
    nudgeRight2 = new QShortcut(QKeySequence("Ctrl+="), shortcutParent);
    if (auto btn = getnudgeRightObject())
    {
        connect(nudgeRight2, &QShortcut::activated, btn, &QToolButton::click);
    }


    // Nudge Left - Ctrl+-
    nudgeLeft = new QShortcut(QKeySequence::ZoomOut, shortcutParent);
    if (auto btn = getnudgeLeftObject())
    connect(nudgeLeft, &QShortcut::activated, btn, &QToolButton::click);

    // Rotate Left - e.g., custom defined macro ROTATE_CCW_KEY
    rotateLeft = new QShortcut(QKeySequence(ROTATE_CCW_KEY), shortcutParent);
    if (auto btn = getRotateLeftObject())
    {
        connect(rotateLeft, &QShortcut::activated, btn, &QToolButton::click);
    }


    // Rotate Right
    rotateRight = new QShortcut(QKeySequence(ROTATE_CW_KEY), shortcutParent);
    if (auto btn = getRotateRightObject())
    {
        connect(rotateRight, &QShortcut::activated,  btn, &QToolButton::click);
    }


    // Turn to bearing
    turnToBearing = new QShortcut(QKeySequence(ROTATE_TURN_KEY), shortcutParent);
    if (auto btn = getRotateButtonObject())
    {
       connect(turnToBearing, &QShortcut::activated,  btn, &QToolButton::click);
    }


    // Stop
    rotateStop = new QShortcut(QKeySequence(ROTATE_STOP_KEY), shortcutParent);
    if (auto btn = getStopRotateObject())
    {
        connect(rotateStop, &QShortcut::activated,  btn, &QToolButton::click);
    }

}



void RotControlFrameBase::setTurnDisplayText(QString brg)
{
    if (auto brglne = getBrgLineEditObject())
    {
       brglne->setText(brg);
    }

}




void RotControlFrameBase::turnTo(int angle)
{
    traceMsg("Turn to - " + QString::number(angle));

    if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        //ui->BrgSt->setText(bearingForDisplay(angle));

        if (rotFrameData.getRotConnected())
        {
            traceMsg("Turn to - Contest match and connected");

            if (angle == COMPASS_ERROR)
            {
                traceMsg(QString("TurnTo: Bearing empty or invalid"));
                QString msg = HtmlFontColour(Qt::red) + tr("Bearing empty or invalid");

                if (auto stateLabel = getRotConnectStateLabelObject())
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

                if (auto stateLabel = getRotConnectStateLabelObject())
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

                if (auto stateLabel = getRotConnectStateLabelObject())
                {
                    stateLabel->setText(msg);
                }

                return;
            }
            else if (angle == rotFrameData.getAntennaBearing())
            {
                traceMsg(QString("TurnTo: Bearing = CurrentBearing"));
                return;
            }
            else
            {
                traceMsg(QString("Send Bearing %1 to Rotator Control").arg(QString::number(angle)));
                emit sendRotator(rpcConstants::eRotateDirect, angle);
                showTurnButOn();
                rotFrameData.setMoving(true);
            }


        }

    }
}

void RotControlFrameBase::onRotate_clicked()
{
    if (rotFrameData.getRotConnected() && !rotFrameData.getRotError())
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
        traceMsg(QString("On Rotate:Rotconnected = %1, RotError = %2").arg(rotFrameData.getRotConnected() ? "True" : "False").arg(rotFrameData.getRotError() ? "True" : "False"));
    }


}

void RotControlFrameBase::onNudgeLeft_clicked()
{
    if (rotFrameData.getRotConnected() && !rotFrameData.getRotError())
    {
        traceMsg("Nudge Left Clicked");

        int newBearing = rotFrameData.getAntennaBearing() - 3;
        if (newBearing < 0)
            newBearing += 360;
        setTurnDisplayText(convertBearingForDisplay(QString::number(newBearing)));
        turnTo(newBearing);
    }
    else
    {
        traceMsg(QString("NudgeLeft:Rotconnected = %1, RotError = %2").arg(rotFrameData.getRotConnected() ? "True" : "False").arg(rotFrameData.getRotError() ? "True" : "False"));
    }
}

void RotControlFrameBase::onNudgeRight_clicked()
{

    if (rotFrameData.getRotConnected() && !rotFrameData.getRotError())
    {
        traceMsg("Nudge Right Clicked");
        int newBearing = rotFrameData.getAntennaBearing() + 3;
        if (newBearing >= 360)
            newBearing -= 360;
        setTurnDisplayText(convertBearingForDisplay(QString::number(newBearing)));
        turnTo(newBearing);
    }
    else
    {
        traceMsg(QString("NudgeRight:Rotconnected = %1, RotError = %2").arg(rotFrameData.getRotConnected() ? "True" : "False").arg(rotFrameData.getRotError() ? "True" : "False"));
    }

}

void RotControlFrameBase::onRotateLeft_clicked()
{

    if (!rotFrameData.getRotConnected() || rotFrameData.getRotError())
    {
        traceMsg(QString("On Rotate Left:Rotconnected = %1, RotError = %2").arg(rotFrameData.getRotConnected() ? "True" : "False").arg(rotFrameData.getRotError() ? "True" : "False"));
        return;
    }

    traceMsg("RotLeft Button Clicked");
    if (rotFrameData.getRotLeftButtonStatus())
    {
        traceMsg("RotLeft Button On - Stop and Turn Off");
        onStopRotate_clicked();
        rot_left_button_off();

    }
    else
    {
        traceMsg("Current Bearing = " + QString::number(rotFrameData.getAntennaBearing()));
        traceMsg("Rotator Bearing = " + QString::number(rotFrameData.getRotatorBearing()));
        traceMsg(QString("RotLeft Status = %1").arg(rotFrameData.getRotLeftButtonStatus() ? "On" : "Off"));
        int angle = 0;

        if (rotFrameData.getRotatorBearing() <= rotFrameData.getRotatorMinAzimuth())
        {
            traceMsg(QString("Current Bearing = %1 <= minAzimuth %2").arg(QString::number(rotFrameData.getRotatorBearing()), QString::number(rotFrameData.getRotatorMinAzimuth())));
            return;
        }

        if (rotFrameData.getMoving() || rotFrameData.getMovingCW() || rotFrameData.getMovingCCW())
        {
            traceMsg("RotLeft Stopping");
            onStopRotate_clicked();
        }

        rot_left_button_on();
        traceMsg("Send RotLeft to Rototor Control");
        emit sendRotator(rpcConstants::eRotateLeft, angle);
        rotFrameData.setMovingCW(true);
    }

}

void RotControlFrameBase::clearBearingLineEdit()
{
    if (auto le = getBrgLineEditObject())
    {
        le->clear();
    }
}


void RotControlFrameBase::onRotateRight_clicked()
{

    if (!rotFrameData.getRotConnected() || rotFrameData.getRotError())
    {
        traceMsg(QString("On Rotate Right:Rotconnected = %1, RotError = %2").arg(rotFrameData.getRotConnected() ? "True" : "False").arg(rotFrameData.getRotError() ? "True" : "False"));
        return;
    }


    traceMsg("RotRight Button Clicked");

    if (rotFrameData.getRotRightButtonStatus())
    {
        traceMsg("RotRight Button On - Stop and Turn Off");
        onStopRotate_clicked();
        rot_right_button_off();

    }
    else
    {
        traceMsg("Current Bearing = " + QString::number(rotFrameData.getAntennaBearing()));
        traceMsg("Rotator Bearing = " + QString::number(rotFrameData.getRotatorBearing()));
        traceMsg(QString("RotRight Status = %1").arg(rotFrameData.getRotRightButtonStatus()));
        int angle = 0;

        if (rotFrameData.getRotatorBearing() >= rotFrameData.getRotatorMaxAzimuth())
        {
            traceMsg(QString("Current Bearing = %1 >= maxAzimuth %2").arg(QString::number(rotFrameData.getAntennaBearing()), QString::number(rotFrameData.getRotatorMaxAzimuth())));
            return;
        }

        if (rotFrameData.getMoving() || rotFrameData.getMovingCW() || rotFrameData.getMovingCCW())
        {
            traceMsg("RotRight Stopping");
            onStopRotate_clicked();
        }

        rot_right_button_on();
        traceMsg("Send RotRight to Rotator Control");
        emit sendRotator(rpcConstants::eRotateRight, angle);
        rotFrameData.setMovingCCW(true);
    }

}



void RotControlFrameBase::rot_left_button_on()
{
    rotFrameData.setRotLeftButtonStatus(true);
    showRotLeftButOn();
}

void RotControlFrameBase::rot_left_button_off()
{
    rotFrameData.setRotLeftButtonStatus(false);
    showRotLeftButOff();
}

void RotControlFrameBase::rot_right_button_on()
{
    rotFrameData.setRotRightButtonStatus(true);
    showRotRightButOn();
}

void RotControlFrameBase::rot_right_button_off()
{
    rotFrameData.setRotRightButtonStatus(false);
    showRotRightButOff();
}


void RotControlFrameBase::showTurnButOn()
{
    //ui->Rotate->setPalette(*redText);

    if (auto tb = getRotateButtonObject())
    {
        tb->setStyleSheet(BUTTON_ON_STYLE);
        tb->setText(tr("Turn"));
    }

}

void RotControlFrameBase::showTurnButOff()
{

    if (auto tb = getRotateButtonObject())
    {
        tb->setStyleSheet(BUTTON_OFF_STYLE);
        tb->setText(tr("Turn"));
    }
}




void RotControlFrameBase::showRotLeftButOn()
{

    if (auto tb = getRotateLeftObject())
    {
        tb->setStyleSheet(BUTTON_ON_STYLE);
        tb->setText(tr("(CCW) Left"));
    }

}

void RotControlFrameBase::showRotLeftButOff()
{

    if (auto tb = getRotateLeftObject())
    {
        tb->setStyleSheet(BUTTON_OFF_STYLE);
        tb->setText(tr("(CCW) Left"));
    }
}

void RotControlFrameBase::showRotRightButOn()
{

    if (auto tb = getRotateRightObject())
    {
        tb->setStyleSheet(BUTTON_ON_STYLE);
        tb->setText(tr("(CW) Right"));
    }
}

void RotControlFrameBase::showRotRightButOff()
{

    if (auto tb = getRotateRightObject())
    {
        tb->setStyleSheet(BUTTON_OFF_STYLE);
        tb->setText(tr("(CW) Right"));
    }
}

void RotControlFrameBase::onStopRotate_clicked()
{
    emit sendRotator(rpcConstants::eRotateStop, 0);
    rotFrameData.clearRotatorMovingFlags();
    showTurnButOff();
    showRotLeftButOff();
    showRotRightButOff();
}




void RotControlFrameBase::clearRotatorIndicators()
{
    rot_left_button_off();
    rot_right_button_off();
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

    if (auto cb = getAntennaSelectObject())
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
        if (sl.count() > 0 && sl[0] != rotFrameData.getLastConnectStat())
        {
            rotFrameData.setLastConnectStat(sl[0]);
            if (rotFrameData.getLastConnectStat() == ROT_STATUS_CONNECTED)
            {

                if (auto lbl = getRotConnectStateLabelObject())
                {
                   lbl->setText(tr("Connected"));
                }

                rotFrameData.setRotError(false);
                rotFrameData.setRotConnected(true);

                setRotatorAntennaName(ct->antennaName.getValue().toString()); // make sure the name appears

                emit rotatorConnected(true);     // tell bandmap
            }
            else if (rotFrameData.getLastConnectStat() == ROT_STATUS_DISCONNECTED)
            {
                if (auto lbl = getRotConnectStateLabelObject())
                {
                    lbl->setText(tr("Disconnected"));
                }
                rotFrameData.setRotError(false);
                rotFrameData.setRotConnected(false);
                emit rotatorConnected(false);     // tell bandmap


            }
        }
        if (sl.count() > 1 && sl[1] != rotFrameData.getLastStatus())
        {
            rotFrameData.setLastStatus(sl[1]);

            if (rotFrameData.getLastStatus() == ROT_STATUS_STOP)
            {
                if (auto lbl = getRotConnectStateLabelObject())
                {
                    lbl->setText(tr("Stop"));
                }

                rotFrameData.setRotError(false);
                rotFrameData.clearRotatorMovingFlags();
                showRotLeftButOff();
                showRotRightButOff();
                showTurnButOff();
            }
            else if (rotFrameData.getLastStatus() == ROT_STATUS_ROTATE_CCW)
            {
                if (auto lbl = getRotConnectStateLabelObject())
                {
                   lbl->setText(HtmlFontColour("Green") + tr("Rotating CCW"));
                }

                rotFrameData.setRotError(false);
                rotFrameData.setMoving(false);
                rotFrameData.setMovingCW(false);
                rotFrameData.setMovingCCW(true);
                // clearRotatorFlags();
                showRotLeftButOn();
            }
            else if (rotFrameData.getLastStatus() == ROT_STATUS_ROTATE_CW)
            {
                if (auto lbl = getRotConnectStateLabelObject())
                {
                    lbl->setText(HtmlFontColour("Green") + tr("Rotating CW"));
                }

                rotFrameData.setRotError(false);
                rotFrameData.setMoving(false);
                rotFrameData.setMovingCW(true);
                rotFrameData.setMovingCCW(false);
                //clearRotatorFlags();
                showRotRightButOn();
            }
            else if (rotFrameData.getLastStatus() == ROT_STATUS_TURN_TO)
            {

               if (auto lbl =  getRotConnectStateLabelObject())
               {
                   lbl->setText(HtmlFontColour("Green") + tr("Turning to bearing"));
               }

                rotFrameData.setRotError(false);
                rotFrameData.setMoving(true);
                rotFrameData.setMovingCW(false);
                rotFrameData.setMovingCCW(false);
                showTurnButOn();
                //clearRotatorFlags();
            }
            else if (rotFrameData.getLastStatus() == ROT_STATUS_CONNECTED)
            {
                if (auto lbl = getRotConnectStateLabelObject())
                {
                    lbl->setText(tr("Connected"));
                }

               rotFrameData.setRotError(false);
               rotFrameData.setRotConnected(true);
            }
            else if (rotFrameData.getLastStatus() == ROT_STATUS_DISCONNECTED)
            {
                if (auto lbl = getRotConnectStateLabelObject())
                {
                    lbl->setText(tr("Disconnected"));
                }

                rotFrameData.setRotError(false);
                rotFrameData.setRotConnected(false);
            }
            else if (rotFrameData.getLastStatus() == ROT_STATUS_ERROR)
            {
                if (auto lbl = getRotatorStatMsgLabelObject())
                {
                    lbl->setText(HtmlFontColour("Red") + tr("Error"));
                }

                rotFrameData.setRotError(true);
            }
            else
            {
                if (auto lbl = getRotatorStatMsgLabelObject())
                {
                    lbl->setText(rotFrameData.getLastStatus());
                }
            }

        }
    }
    if (sl.count() <= 1)     // will be a revoked state
    {
        if (auto lbl = getRotConnectStateLabelObject())
        {
            lbl->setText(tr("Disconnected"));
        }

        rotFrameData.setRotError(false);
        rotFrameData.setRotConnected(false);
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

    rotFrameData.setAntennaName(cb->currentData().toString());
    if (ct && !ct->isReadOnly())
    {
        emit selectRotator(rotFrameData.getAntennaName());
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

    if (auto lbl = getRotBrgDisplayObject())
    {
        displayCompassBearingWithOverlap(s, rotFrameData, lbl);
    }



}

void RotControlFrameBase::setRotatorMaxAzimuth(const int maxAz)
{
    traceMsg(QString("Set MaxAzimuth = %1").arg(QString::number(maxAz)));

    rotFrameData.setRotatorMaxAzimuth(maxAz);

}


void RotControlFrameBase::setRotatorMinAzimuth(const int minAz)
{
    traceMsg(QString("Set MinAzimuth = %1").arg(QString::number(minAz)));

    rotFrameData.setRotatorMinAzimuth(minAz);

}

void RotControlFrameBase::setSupportStopCommandFlag(bool state)
{
    traceMsg((QString("Set Support Stop Command Flag = %1").arg(state ?  "True" : "False" )));
    rotFrameData.setSupportStopCommand(state);

    if (auto tb = getStopRotateObject())
    {
        tb->setVisible(state);
    }

}


void RotControlFrameBase::setCwCcwCmdEnable(bool s)
{
    rotFrameData.SetSupportCwCcwCmd(s);
    setCwCcW_Items_Visible(s);
}

void RotControlFrameBase::setCwCcW_Items_Visible(bool visible)
{
    auto tb = getnudgeLeftObject();
    if (tb)
    {
       tb->setVisible(visible);
    }

    tb = getnudgeRightObject();
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

    if (auto gb = getRotatorControlGroupBoxObject())
    {
        gb->setDisabled(disabled);
    }


}

void RotControlFrameBase::traceMsg(QString msg)
{
    trace(QString("[%1] %2 - %3").arg(rotFrameData.getFrameName(), rotFrameData.getAntennaName(), msg));
}

void RotControlFrameBase::onAntennaNameSel_activated(int /*arg1*/)
{
    QComboBox* cb = getAntennaSelectObject();
    if (cb)
    {
        rotFrameData.setAntennaName(cb->currentData().toString());
    }


    emit selectRotator(rotFrameData.getAntennaName());

}
void RotControlFrameBase::getRotDetails(memoryData::memData &m)
{
    m.bearing = rotFrameData.getAntennaBearing();;
}

void RotControlFrameBase::presetTurn(QString b)
{
    if (!b.isEmpty())
    {
        turnTo(b.toInt());
        //ui->BrgSt->setText(b);
        setTurnDisplayText(convertBearingForDisplay(b));

        if (auto le = getBrgLineEditObject())
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

    // can not remember what this was for!
    bool test = false;
    if (auto le = getBrgLineEditObject())
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

void RotControlFrameBase::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    activate();
}

void RotControlFrameBase::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    deactivate();
}


void RotControlFrameBase::activate()
{
    activateSignalConnections();
}
void RotControlFrameBase::deactivate()
{
    deactivateSignalConnections();
}


void RotControlFrameBase::activateSignalConnections()
{
    if (auto ui_element = getStopRotateObject())
    {
        connect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onStopRotate_clicked);
    }

    if (auto ui_element = getRotateButtonObject())
    {
        connect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onRotate_clicked);
    }

    if (auto ui_element = getRotateLeftObject())
    {
        connect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onRotateLeft_clicked);
    }

    if (auto ui_element = getRotateRightObject())
    {
        connect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onRotateRight_clicked);
    }

    if (auto ui_element = getnudgeLeftObject())
    {
        connect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onNudgeLeft_clicked);
    }

    if (auto ui_element = getnudgeRightObject())
    {
        connect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onNudgeRight_clicked);
    }

    if (auto ui_element = getBrgLineEditObject())
    {
        connect(ui_element, &BearingLineEdit::editingFinished, this, &RotControlFrameBase::onRotate_clicked);
    }

    if (auto ui_element = getAntennaSelectObject())
    {
        connect(ui_element, QOverload<int>::of(&QComboBox::activated), this, &RotControlFrameBase::onAntennaNameSel_activated);
    }

    if (auto ui_element = qobject_cast<BearingLineEdit*>(getBrgLineEditObject()))
    {
        connect(ui_element, &BearingLineEdit::editingFinished, this, &RotControlFrameBase::onRotate_clicked);
    }



    //connect(this, &RotControlFrameBase::bearingEditReturn, this, &RotControlFrameBase::onRotate_clicked);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::BrgStrToRot, this, &RotControlFrameBase::setBrgFromQSOLog);

    // from match frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MatchBrgStrToRot, this, &RotControlFrameBase::setBrgFromMatchFrame);

    // from cluster frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SpotBrgStrToRot, this, &RotControlFrameBase::setBrgFromSpot);

    // from memory frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MemBrgStrToRot, this, &RotControlFrameBase::setBrgFromFrmMemory);
}

void RotControlFrameBase::deactivateSignalConnections()
{
    if (auto ui_element = getStopRotateObject())
    {
        disconnect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onStopRotate_clicked);
    }

    if (auto ui_element = getRotateButtonObject())
    {
        disconnect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onRotate_clicked);
    }

    if (auto ui_element = getRotateLeftObject())
    {
        disconnect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onRotateLeft_clicked);
    }

    if (auto ui_element = getRotateRightObject())
    {
        disconnect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onRotateRight_clicked);
    }

    if (auto ui_element = getnudgeLeftObject())
    {
        disconnect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onNudgeLeft_clicked);
    }

    if (auto ui_element = getnudgeRightObject())
    {
        disconnect(ui_element, &QToolButton::clicked, this, &RotControlFrameBase::onNudgeRight_clicked);
    }

    if (auto ui_element = getBrgLineEditObject())
    {
        disconnect(ui_element, &BearingLineEdit::editingFinished, this, &RotControlFrameBase::onNudgeRight_clicked);
    }

    if (auto ui_element = getAntennaSelectObject())
    {
        disconnect(ui_element, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RotControlFrameBase::onAntennaNameSel_activated);
    }

    if (auto ui_element = qobject_cast<BearingLineEdit*>(getBrgLineEditObject()))
    {
        disconnect(ui_element, &BearingLineEdit::editingFinished, this, &RotControlFrameBase::onRotate_clicked);
    }



    //connect(this, &RotControlFrameBase::bearingEditReturn, this, &RotControlFrameBase::onRotate_clicked);

    disconnect(&MinosLoggerEvents::mle, &MinosLoggerEvents::BrgStrToRot, this, &RotControlFrameBase::setBrgFromQSOLog);

    // from match frame
    disconnect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MatchBrgStrToRot, this, &RotControlFrameBase::setBrgFromMatchFrame);

    // from cluster frame
    disconnect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SpotBrgStrToRot, this, &RotControlFrameBase::setBrgFromSpot);

    // from memory frame
    disconnect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MemBrgStrToRot, this, &RotControlFrameBase::setBrgFromFrmMemory);
}

void RotControlFrameBase::checkConnection()
{
    QString loggerUuid = LogContainer->sendDM->getLoggerUuid();
    PubSubName rotSelected = LogContainer->sendDM->getSelectedRot(loggerUuid);

    if (rotSelected.isEmpty() || !isRotatorLoaded())
    {
        // clear the rot selection


        if (auto cb = getAntennaSelectObject())
        {
           cb->setCurrentText("");
        }

        setRotatorState(ROT_STATUS_DISCONNECTED);
    }
}


void RotControlFrameBase::setFrameName(QString frameName)
{
    rotFrameData.setFrameName(frameName);
}
