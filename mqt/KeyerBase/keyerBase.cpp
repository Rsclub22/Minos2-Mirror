#include <QMap>
#include "keyerBase.h"

//==============================================================================

QMap <int, QString> MORSECODE;    // . is 0x40, - is 0x80
QMap <int, MORSEMSG> MORSEMSGS;

//==============================================================================

// some of these might not belong here, but they were in sbdvp or tlkeyer
// and I still need them somewhere
bool sblog = false;
unsigned long MORSEINTCOUNT = 0;

int tuneTime = 10;
double tuneLevel = 80.0;

int CWTone = 1000;
int CWSpeed = 12;

qint64 currTick;
my_deque < KeyerAction *> KeyerAction::currentAction;
