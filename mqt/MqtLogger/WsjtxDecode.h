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
    QString getMStage() const;
    SpecialOperatingActivity opMode = NONE;

    QTime decodeTime;
    QString message;
    bool best = false;
    bool autoresp = false;
    Callsign toCall;
    Locator toGrid;
    Callsign fromCall;
    Locator fromGrid;
    int strength = -100;
    int bearing = 0;
    int distance = 0;
    int points = 0;

    // extra info needed for reply

    QString id;
    QTime time;
    qint32 snr;
    float delta_time;
    quint32 delta_frequency;
    QString mode;
    bool low_confidence;
    bool off_air;

    decodeMessage();
    bool checkAsContact();
};

class WsjtxDecode
{
public:
    WsjtxDecode();

    decodeMessage decode(QString const& id, QTime time, qint32 snr, float delta_time
                         , quint32 delta_frequency, QString const& mode
                         , QString const& message_text, bool low_confidence, bool off_air);

    Callsign getMyCall() const;
    void setMyCallGrid(const QString &c, const QString &l);
    Locator getMyGrid() const;
    void setMyGrid(const QString &value);
private:
    Callsign myCall;
    Locator myGrid;

};

#endif // WSJTXDECODE_H
