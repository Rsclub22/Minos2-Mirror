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


#include "LoggerContest.h"
#include "rotatorskyscanframe.h"
#include "ui_rotatorskyscanframe.h"


RotatorSkyScanFrame::RotatorSkyScanFrame(QWidget *parent):
    QFrame(parent),
    ct(nullptr),
    ui(new Ui::RotatorSkyScanFrame)
{

    ui->setupUi(this);

}


RotatorSkyScanFrame::~RotatorSkyScanFrame()
{
    delete ui;

}



void RotatorSkyScanFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

}

void RotatorSkyScanFrame::on_ContestPageChanged()
{




}
