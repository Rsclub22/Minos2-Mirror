#include "WsjtxDecode.h"
#include "ScreenContact.h"
#include "contest.h"
#include "rigutils.h"

/*
enum MessageStage {emsNone, emsCQ, emsGrid, emsDb, emsRplusDb, emsRRR, ems73, emsFree};

// Normal
CQ K1ABC FN42                          #K1ABC calls CQ                 emsCQ
                  K1ABC G0XYZ IO91     #G0XYZ answers                  emsGrid
G0XYZ K1ABC –19                        #K1ABC sends report             emsDB
                  K1ABC G0XYZ R-22     #G0XYZ sends R+report           emsRplusDb
G0XYZ K1ABC RR73                       #K1ABC sends RRR - or RR73      emsRRR
                  K1ABC G0XYZ 73       #G0XYZ sends 73                 ems73


// EU VHF Contest

CQ TEST G4ABC/P IO91                                                   emsCQ
                                   G4ABC/P PA9XYZ JO22                 emsGrid
PA9XYZ 590003 IO91NP                                                   emsDB
                                   G4ABC/P R 570007 JO22DB             emsRplusDb
PA9XYZ G4ABC/P RR73                                                    emsRRR
                                   G4ABC/P PA9XYZ 73                   ems73

And in 2.2 (both callsigns, hashed)
(Either callsign (or both) may have /P appended.)

CQ TEST G4ABC IO91
                                G4ABC PA9XYZ JO22
<PA9XYZ> <G4ABC> 570123 IO91NP
                                <G4ABC> <PA9XYZ> R 580071 JO22DB
PA9XYZ G4ABC RR73


// NA VHF Contest
(Either callsign (or both) may have /R appended. You can use RR73 in place of RRR, and the final 73 is optional)

CQ TEST K1ABC FN42                                                     emsCQ
                     K1ABC W9XYZ EN37                                  emsGrid
W9XYZ K1ABC R FN42                                                     emsRplusGrid
                     K1ABC W9XYZ RR73                                  emsRRR
W9XYZ K1ABC 73                                                         ems73

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
    if (c.fullCall.getValue() != "..." && c.valRes == CS_OK && (l.valRes == LOC_OK || l.valRes == LOC_PARTIAL) )
    {
        if (GridCallMap[c.realCall].valRes != LOC_OK)
            GridCallMap[c.realCall] = l;
    }
}

const Locator &WsjtGetCallLoc(const Callsign &c)
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
//enum MessageStage {emsNone, emsCQ, emsGrid, emsDb, emsDbGrid, emsRplusGrid, emsRplusDb, emsRplusDbGrid, emsRRR, ems73, emsFree};

    switch (mstage)
    {
    case emsNone:
        return tr("None");
    case emsCQ:
        return "CQ";
    case emsGrid:
        return tr("Grid");
    case emsDb:
        return "db";
    case emsDbGrid:
        return "Db+grid";
    case emsRplusDb:
        return "R+db";
    case emsRplusGrid:
        return "R+grid";
    case emsRplusDbGrid:
        return "R+db+grid";
    case emsRRR:
        return "RRR";
    case ems73:
        return "73";
    case emsFree:
        return tr("Free");
    }
    return "N/A";
}
bool decodeMessage::checkAsContact()
{
    BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
    if (!cc)
        return false;

    if (txrx == eTX)
        return false;

    ScreenContact scc;
    scc.initialise(cc);
    scc.cs = fromCall;
    scc.loc = fromGrid;
    scc.time = dtg(true);
    QString cb;
    scc.frequency = convertFreqToStr(static_cast<int>(cc->getTxFreqBand("", cb)));

    scc.checkScreenContact();
    csret = scc.cs.valRes;
    if (scc.screenQSOValid)
    {
        scc.score();
        points = scc.contactScore;
        distance = points;
        bearing = scc.bearing;
        mults = 0;
        if (scc.multCount >= 1)
        {
            mults = scc.multCount;
        }
        if (scc.bonus)
        {
            bonus = scc.bonus;
        }
        return true;
    }

    return false;
}
void decodeMessage::validate()
{
    fromCall.validate();
    toCall.validate();
    fromGrid.validate();

    if (fromCall.fullCall.getValue() == "...")
    {
        fromCall.valRes = CS_OK;
    }
    if (toCall.fullCall.getValue() == "...")
    {
        toCall.valRes = CS_OK;
    }
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
QString WsjtxDecode::stripBrackets(QString cs)
{
    cs.replace("<", "");
    cs.replace(">", "");
    return cs;
}
decodeMessage WsjtxDecode::decode(const QString &id, TxRx tr, QTime time, qint32 snr, float delta_time, quint32 delta_frequency,
                                  const QString &mode, const QString &message_text, bool low_confidence, bool off_air)
{
    decodeMessage dc;

    dc.id = id;
    dc.txrx = tr;
    dc.time = time;
    dc.snr = snr;
    dc.delta_time = delta_time;
    dc.delta_frequency = delta_frequency;
    dc.mode = mode;
    dc.low_confidence = low_confidence;
    dc.off_air = off_air;
    
    // there shouldn't be any spare spaces unless there is a decode indicator

    int spoff = message_text.trimmed().indexOf("      ");
    if (spoff > 0)
    {
        dc.message = message_text.left(spoff + 1).trimmed();
        // looking for e.g.
        // G0GJV GM8MJV IO85                   ? a3
        // G0GJV GM8MJV IO85                     a3
        dc.decodeInd = message_text.mid(spoff + 2).trimmed();
    }
    else
    {
        dc.message= message_text.trimmed();
    }

    QStringList sl = message_text.trimmed().split(' ', QString::SkipEmptyParts);

    if (sl.count() == 0)
        return dc;

    if  (sl[0] == "CQ" || sl[0] == "QRZ")
    {
        // CQ K1ABC FN42
        // CQ DX K1ABC FN42
        // CQ TEST G4ABC/P IO91
        dc.mstage = emsCQ;
        int callIndex = 1;

        if (sl.count() == 4)
        {
            dc.CQCall = sl[0] + " " + sl[1];
            callIndex = 2;
        }
        else
        {
            dc.CQCall = sl[0];
        }

        // NB we can get bad decodes
        if (callIndex + 1 < sl.count())
        {
            dc.fromCall = Callsign(stripBrackets(sl[callIndex]));
            dc.fromGrid = Locator(sl[callIndex + 1]);
            dc.validate();
            addCall(dc.fromCall, dc.fromGrid);
        }
    }
    else if (sl.count() == 3 && (sl[2] == "RR73" || sl[2] == "RRR"))
    {
        // G0XYZ K1ABC RR73
        // PA9XYZ G4ABC/P RR73
        dc.mstage = emsRRR;
        dc.toCall = Callsign(stripBrackets(sl[0]));
        dc.fromCall = Callsign(stripBrackets(sl[1]));
        dc.validate();

    }
    else if (sl.count() == 3 && sl[2] == "73")
    {
        // K1ABC G0XYZ 73
        // G4ABC/P PA9XYZ 73
        dc.mstage = ems73;
        dc.toCall = Callsign(stripBrackets(sl[0]));
        dc.fromCall = Callsign(stripBrackets(sl[1]));
        dc.validate();

    }
    else if (sl.count() == 3 && sl[2].indexOf('R') == 0)
    {
        // K1ABC G0XYZ R-22
        dc.mstage = emsRplusDb;
        dc.toCall = Callsign(stripBrackets(sl[0]));
        dc.fromCall = Callsign(stripBrackets(sl[1]));
        dc.validate();

    }
    else if (sl.count() == 4 && sl[2] == "R")
    {
        // G0GJV 2E0EVM R IO80
        dc.mstage = emsRplusGrid;
        dc.toCall = Callsign(stripBrackets(sl[0]));
        dc.fromCall = Callsign(stripBrackets(sl[1]));
        dc.fromGrid = Locator(sl[3]);
        dc.validate();

    }
    else if (sl.count() == 5 && sl[2] == "R")
    {
       // <G4ABC> <PA9XYZ> R 580071 JO22DB
        dc.mstage = emsRplusDbGrid;
        dc.toCall = Callsign(stripBrackets(sl[0]));
        dc.fromCall = Callsign(stripBrackets(sl[1]));
        dc.fromGrid = Locator(sl[4]);
        dc.validate();
        addCall(dc.fromCall, dc.fromGrid);
    }
    else if (sl.count() == 4 && sl[1] == "R")
    {
        // this will disappear with wsjt-x 2.2
        // G4ABC/P R 570007 JO22DB
        dc.mstage = emsRplusDbGrid;
        dc.toCall = Callsign(stripBrackets(sl[0]));
        dc.fromGrid = Locator(sl[3]);
        dc.validate();
        addCall(dc.fromCall, dc.fromGrid);

        // how do we get fromCall? From previous grid, db calls?
        // this is why it is changing...
    }
    else
    {
        // now we are left with grid or db or dbGrid
        // grid is sl[2] is 4 fig loc

        // grid
        // K1ABC G0XYZ IO91
        // G4ABC/P PA9XYZ JO22

        // db
        //G0XYZ K1ABC –19
        //G0XYZ K1ABC 2

        // dbGrid
        //PA9XYZ 590003 IO91NP
        //<PA9XYZ> <G4ABC> 570123 IO91NP

        dc.toCall = Callsign(stripBrackets(sl[0]));
        dc.toCall.validate();
        if (dc.toCall.fullCall.getValue() == "...")
        {
            dc.toCall.valRes = CS_OK;
        }

        Callsign c0(stripBrackets(sl[0]));
        Callsign c1(stripBrackets(sl[1]));
        Locator l1(sl[2]);

        if (sl.count() == 3)
        {
            if (isNumeric(sl[1]))
            {
                // old style EU VHF
                dc.toCall = c0;
                dc.mstage = emsDbGrid;
                dc.fromGrid = l1;
                dc.validate();
                addCall(dc.fromCall, dc.fromGrid);
            }
            else
            {
                dc.toCall = c0;
                dc.fromCall = c1;
                QChar sl20 = sl[2][0];
                if (sl20.isLetter())
                {
                    dc.fromGrid = l1;
                    dc.mstage = emsGrid;
                    dc.validate();
                    addCall(dc.fromCall, dc.fromGrid);
                }
                else if (sl20.isDigit() || (sl20 == QChar('-')))
                {
                    dc.validate();
                    dc.mstage = emsDb;
                }
                else
                {
                    dc.mstage = emsFree;
                }
            }
        }
        else if (sl.count() == 4)
        {
            // new style EU VHF
            dc.toCall = c0;
            dc.fromCall = c1;
            dc.fromGrid = l1;
            dc.validate();
            dc.mstage = emsDbGrid;
        }
    }

    if (dc.fromCall.valRes == CS_OK && dc.fromGrid.valRes != LOC_OK && dc.fromGrid.valRes != LOC_PARTIAL)
    {
        dc.fromGrid = WsjtGetCallLoc(dc.fromCall);
    }
    if (dc.toCall.valRes == CS_OK)
    {
        // we never have toGrid
        dc.toGrid = WsjtGetCallLoc(dc.toCall);
    }
    if (dc.fromCall.realCall != myCall.realCall)
    {
        dc.checkAsContact();
    }
    return dc;
}
