#include "WsjtxDecode.h"
#include "ScreenContact.h"
#include "contest.h"

/*
enum MessageStage {emsNone, emsCQ, emsGrid, emsDb, emsRplusDb, emsRRR, ems73, emsFree};

  // Normal
CQ K1ABC FN42                          #K1ABC calls CQ                  emsCQ
                  K1ABC G0XYZ IO91     #G0XYZ answers                   emsGrid
G0XYZ K1ABC –19                        #K1ABC sends report              emsDB
                  K1ABC G0XYZ R-22     #G0XYZ sends R+report            emsRplusDb
G0XYZ K1ABC RR73                       #K1ABC sends RRR - or RR73       emsRRR
                  K1ABC G0XYZ 73       #G0XYZ sends 73                  ems73
  */

/*
  // EU VHF Contest
CQ TEST G4ABC/P IO91                                                    emsCQ
                                   G4ABC/P PA9XYZ JO22                  emsGrid
PA9XYZ 590003 IO91NP                                                    emsDB
                                   G4ABC/P R 570007 JO22DB              emsRplusDb
PA9XYZ G4ABC/P RR73                                                     emsRRR
                                   G4ABC/P PA9XYZ 73                    ems73

 PA9XYZ 590003 IO91NP (emsDb)

 Look for previous grid message from PA9XYZ, which gives "CQ" call and PA9XYZ partial locator

 Look for previous CQ message for "CQ" call - check DF is in range and locator is compatible
 (as otherwise it could be someone else who is sending the DB message)

 G4ABC/P R 570007 JO22DB (emsRplusDb)

 This gives the CQ call
 Look for previous grid message on frequency with matching CQ call and matching partial loc; this gives responding call
 Intervening should be a DB message to responder from CQ call


  */

/*
  Feed in decoded message
  Break it into component words
  Analyse the message type, and the components
  Try to go backup the message stack to find it's predecessor - at least if we are involved
*/

static QMap<QString, Locator> GridCallMap;
static Locator nullLoc;

static void addCall(const Callsign &c, const Locator &l)
{
    if (c.valRes == CS_OK && (l.valRes == LOC_OK || l.valRes == LOC_PARTIAL) )
    {
        GridCallMap[c.realCall] = l;
    }
}

static const Locator &getCallLoc(const Callsign &c)
{
    if (GridCallMap.contains(c.realCall))
    {
        return GridCallMap[c.realCall];
    }
    return nullLoc;
}

decodeMessage::decodeMessage()
{

}
QString decodeMessage::getMStage() const
{
//    enum MessageStage {emsNone, emsCQ, emsGrid, emsDb, emsRplusDb, emsRRR, ems73, emsFree};
    switch (mstage)
    {
    case emsNone:
        return "None";
    case emsCQ:
        return "CQ";
    case emsGrid:
        return "Grid";
    case emsDb:
        return "db";
    case emsRplusDb:
        return "R+db";
    case emsRRR:
        return "RRR";
    case ems73:
        return "73";
    case emsFree:
        return "Free";
    }
    return "N/A";
}
bool decodeMessage::checkAsContact()
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    ScreenContact scc;
    scc.initialise(cc);
    scc.cs = fromCall;
    scc.loc = fromGrid;
    scc.time = dtg(true);

    scc.checkScreenContact();
    csret = scc.cs.valRes;
    if (scc.screenQSOValid)
    {
        scc.score();
        points = scc.contactScore;
        distance = points;
        bearing = scc.bearing;
        if (scc.multCount >= 1)
        {
            ContestScore cs(cc);
            cc->getScoresTo(cs, QDateTime::currentDateTime());
            int ctmultct = cs.nmults;

            /*
             sum is (new points)*(new mults) - (old points) * (old mults)

             which reduces to the following calculation
             */
            points = (cs.contestScore + points) * ctmultct + points * cs.nmults;
        }
        if (scc.bonus)
        {
            points += scc.bonus;
        }
        return true;
    }

    return false;
}

void WsjtxDecode::setMyCallGrid(const QString &c, const QString &l)
{
    myCall = Callsign(c);
    myCall.validate();

    myGrid = Locator(l);
    myGrid.validate();

    addCall(myCall, myGrid);
}

Callsign WsjtxDecode::getMyCall() const
{
    return myCall;
}

Locator WsjtxDecode::getMyGrid() const
{
    return myGrid;
}
WsjtxDecode::WsjtxDecode()
{
    
}
static bool isNumeric ( const QString &s )
{
    int slen = s.length();
    if ( slen == 0 )
    {
        return false;
    }

    if (s[0] != '+' && s[0] != '-' && !s[0].isNumber())
        return false;

    for ( int i = 1; i < slen; i++ )
    {
        if ( !s[ i ].isNumber() )
        {
            return false;
        }
    }
    return true;
}
decodeMessage WsjtxDecode::decode(const QString &id, QTime time, qint32 snr, float delta_time, quint32 delta_frequency,
                                  const QString &mode, const QString &message_text, bool low_confidence, bool off_air)
{
    decodeMessage dc;

    dc.id = id;
    dc.time = time;
    dc.snr = snr;
    dc.delta_time = delta_time;
    dc.delta_frequency = delta_frequency;
    dc.mode = mode;
    dc.message= message_text.trimmed();
    dc.low_confidence = low_confidence;
    dc.off_air = off_air;
    
    // there shouldn't be any spare spaces but...

    QStringList sl = message_text.trimmed().split(' ', QString::SkipEmptyParts);

    if (sl[0] == "CQ")
    {
        // repliable to "from"
        // CQ K1ABC FN42
        // CQ DX K1ABC FN42
        // CQ TEST G4ABC/P IO91
        dc.mstage = emsCQ;
        int callIndex = 1;
        dc.opMode = NONE;

        if (sl.count() == 4)
        {
            callIndex = 2;
            if (sl[1] == "TEST")
            {
                dc.opMode = EU_VHF;
            }
        }

        // NB we can get bad decodes
        if (callIndex + 1 < sl.count())
        {
            dc.fromCall = Callsign(sl[callIndex]);
            dc.fromCall.validate();
            dc.fromGrid = Locator(sl[callIndex + 1]);
            dc.fromGrid.validate();
            addCall(dc.fromCall, dc.fromGrid);
        }
    }
    else if (sl.count() == 3 && (sl[2] == "RR73" || sl[2] == "RRR"))
    {
        // repliable to "from"
        // G0XYZ K1ABC RR73
        // PA9XYZ G4ABC/P RR73
        dc.mstage = emsRRR;
        dc.toCall = Callsign(sl[0]);
        dc.fromCall = Callsign(sl[1]);
    }
    else if (sl.count() == 3 && sl[2] == "73")
    {
        // repliable to "from"
        // K1ABC G0XYZ 73
        // G4ABC/P PA9XYZ 73
        dc.mstage = ems73;
        dc.toCall = Callsign(sl[0]);
        dc.fromCall = Callsign(sl[1]);
    }
    else if (sl.count() == 3 && sl[2].indexOf('R') == 0)
    {
        // repliable to "from" - but may have to wait for 73
        // K1ABC G0XYZ R-22
        // G4ABC/P R 570007 JO22DB
        dc.mstage = emsRplusDb;
        dc.toCall = Callsign(sl[0]);
        dc.fromCall = Callsign(sl[1]);
    }
    else if (sl.count() == 4 && sl[1] == "R")
    {
        // repliable to "from" - but may have to wait for 73
        // K1ABC G0XYZ R-22
        // G4ABC/P R 570007 JO22DB
        dc.mstage = emsRplusDb;
        dc.opMode = EU_VHF;
        dc.toCall = Callsign(sl[0]);
        dc.fromGrid = Locator(sl[3]);

        //how do we get fromCall? From previous grid, db calls?
    }
    else
    {
        // now we are left with grid or db
        // grid is sl[2] is 4 fig loc
        // K1ABC G0XYZ IO91
        // G4ABC/P PA9XYZ JO22

        // db is aything else? or it may be a free text message
        //G0XYZ K1ABC –19
        //PA9XYZ 590003 IO91NP

        dc.toCall = Callsign(sl[0]);
        dc.toCall.validate();

        if (sl.count() != 3 || dc.toCall.valRes != CS_OK)
        {
            dc.mstage = emsFree;
        }
        else
        {
            Callsign c(sl[1]);
            Locator l1(sl[2]);

            c.validate();
            l1.validate();

            if (c.valRes == CS_OK && (l1.valRes == LOC_OK || l1.valRes == LOC_PARTIAL))
            {
                // repliable to "from" if we are "to"
                // K1ABC G0XYZ IO91
                // G4ABC/P PA9XYZ JO22
                dc.mstage = emsGrid;
                dc.fromCall = c;
                dc.fromCall.validate();
                dc.fromGrid = l1;
                dc.fromGrid.validate();
                addCall(dc.fromCall, dc.fromGrid);
            }
            else if (isNumeric(sl[1])
                     && l1.valRes == LOC_OK)
            {
                // not repliable
                //PA9XYZ 590003 IO91NP
                dc.mstage = emsDb;
                dc.opMode = EU_VHF;
                dc.fromGrid = l1;

                // who is fromCall? need previous CQ, grid
            }
            else if (c.valRes == CS_OK
                     && isNumeric(sl[2]))
            {
                // not repliable
                //G0XYZ K1ABC –19
                dc.mstage = emsDb;
                dc.fromCall = c;
            }
            else
            {
                dc.mstage = emsFree;
            }
        }
    }
    dc.fromCall.validate();
    dc.toCall.validate();
    dc.fromGrid.validate();

    if (dc.fromCall.valRes == CS_OK && dc.fromGrid.valRes != LOC_OK && dc.fromGrid.valRes != LOC_PARTIAL)
    {
        dc.fromGrid = getCallLoc(dc.fromCall);
    }
    if (dc.toCall.valRes == CS_OK)
    {
        // we never have toGrid
        dc.toGrid = getCallLoc(dc.toCall);
    }
    if (dc.fromCall.realCall != myCall.realCall)
    {
        dc.checkAsContact();
    }
    return dc;
}
