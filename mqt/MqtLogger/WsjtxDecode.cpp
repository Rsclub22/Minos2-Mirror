#include "WsjtxDecode.h"
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
  */

/*
  Feed in decoded message
  Break it into component words
  Analyse the message type, and the components
  Try to go backup the message stack to find it's predecessor - at least if we are involved

  */

decodeMessage::decodeMessage()
{

}

QString WsjtxDecode::getMyCall() const
{
    return myCall;
}

void WsjtxDecode::setMyCall(const QString &value)
{
    myCall = value;
}

QString WsjtxDecode::getMyGrid() const
{
    return myGrid;
}

void WsjtxDecode::setMyGrid(const QString &value)
{
    myGrid = value;
}

WsjtxDecode::WsjtxDecode()
{
    
}

decodeMessage WsjtxDecode::decode(QString mess)
{
    decodeMessage dc;
    
    // there shouldn't be any spare spaces but...
    mess = mess.trimmed();
    QStringList sl = mess.split(' ', QString::SkipEmptyParts);

    if (sl[0] == "CQ")
    {
        //CQ message
        dc.mstage = emsCQ;
        dc.opMode = (sl[1] == "TEST")?EU_VHF:NONE;
    }
    else if (sl[2] == "RR73")
    {
        dc.mstage = emsRRR;
    }
    else if (sl[2] == "73")
    {
        dc.mstage = ems73;
    }
    else if (sl[2].indexOf('R') == 0)
    {
        dc.mstage = emsRplusDb;
    }
    // now we are left with grid or db
    // grid is sl[2] is 4 fig loc
    // db is aything else? or it may be a free text message

    return dc;
}
