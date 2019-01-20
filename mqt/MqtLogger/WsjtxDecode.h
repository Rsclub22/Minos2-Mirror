#ifndef WSJTXDECODE_H
#define WSJTXDECODE_H
#include "base_pch.h"

enum MessageStage {emsNone, emsCQ, emsGrid, emsDb, emsRplusDb, emsRRR, ems73, emsFree};
enum SpecialOperatingActivity {NONE, NA_VHF, EU_VHF, FIELD_DAY, RTTY, FOX, HOUND};

class WsjtxFrame;
class decodeMessage
{
public:
    // Can I populate this accurately?
    // Do I need the whole decode set to allow for reply?
    // Doesn't contain MY sent messages... In fact, once working someone
    // it may all go to pot

    // I CAN see transmit "sessions" - does the status give me enough?

    MessageStage mstage{emsNone};
    SpecialOperatingActivity opMode = NONE;

    QTime decodeTime;
    QString message;
    QString toCall;
    QString toGrid;
    QString fromCall;
    QString fromGrid;
    int strength = -100;
    int bearing = 0;
    int distance = 0;
    int points = 0;

    decodeMessage();
};

class WsjtxDecode
{
    QString myCall;
    QString myGrid;
public:
    WsjtxDecode();

    decodeMessage decode(QString);

    QString getMyCall() const;
    void setMyCall(const QString &value);
    QString getMyGrid() const;
    void setMyGrid(const QString &value);
};

#endif // WSJTXDECODE_H
