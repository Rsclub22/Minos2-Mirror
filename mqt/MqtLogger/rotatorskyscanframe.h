/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator SkyScan Frame
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef ROTATORSKYSCANFRAME_H
#define ROTATORSKYSCANFRAME_H

#include <QFrame>
#include <QObject>
#include "RPCCommandConstants.h"
#include "MinosLoggerEvents.h"


namespace Ui {
class RotatorSkyScanFrame;
}





class RotatorSkyScanFrame : public QFrame
{
    Q_OBJECT
    LoggerContestLog *ct = nullptr;
public:
    explicit RotatorSkyScanFrame(QWidget *parent);
    ~RotatorSkyScanFrame();



    void setContest(BaseContestLog *c);
    void on_ContestPageChanged();
private:

    Ui::RotatorSkyScanFrame *ui;
};

#endif // ROTATORSKYSCANFRAME_H
