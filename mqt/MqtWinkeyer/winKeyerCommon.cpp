/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#include "winKeyerCommon.h"



WinkeyerState::WinkeyerState(const WinkeyerState &other)
{
    copy(other);
}


WinkeyerState& WinkeyerState::operator=(const WinkeyerState &other)
{
    if (this != &other)
    {
        copy(other);
    }
    return *this;
}

void WinkeyerState::copy(const WinkeyerState &other)
{
    modereg = other.modereg;
    speed = other.speed;
    stconst = other.stconst;
    weight = other.weight;
    leadin = other.leadin;
    tail = other.tail;
    minwpm = other.minwpm;
    wpmrange = other.wpmrange;
    xtnd = other.xtnd;
    kcomp = other.kcomp;
    farns = other.farns;
    sampadj = other.sampadj;
    ditdahratio = other.ditdahratio;
    pincfg = other.pincfg;
    x1mode = other.x1mode;
}

void WinkeyerState::setDefaults()
{
    setModereg(DEFAULT_MODEREG);
    setSpeed(DEFAULT_SPEED);
    setStconst(DEFAULT_STCONST);
    setWeight(DEFAULT_WEIGHT);
    setLeadin(DEFAULT_LEADIN);
    setTail(DEFAULT_TAIL);
    setMinwpm(DEFAULT_MINWPM);
    setWpmrange(DEFAULT_WPMRANGE);
    setXtnd(DEFAULT_XTND);
    setKcomp(DEFAULT_KCOMP);
    setFarns(DEFAULT_FARNS);
    setSampadj(DEFAULT_SAMPADJ);
    setDitdahratio(DEFAULT_DITDAHRATIO);
    setPincfg(DEFAULT_PINCFG);
    setX1mode(DEFAULT_X1MODE);



}


