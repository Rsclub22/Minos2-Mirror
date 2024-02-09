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
my_deque < KeyerAction > KeyerAction::currentAction;

/*static*/ QSharedPointer<KeyerAction> KeyerAction::getCurrentAction()
{
   if ( currentAction.begin() == currentAction.end() )
      return nullptr;
   return *(currentAction.begin());
}
QSharedPointer<KeyerAction> KeyerAction::getNextAction()
{
   return (KeyerAction::currentAction).next_element( *this );
}
/*
-- Variable strings

-- $MyCall, $HisCall, $CorrectedCall, $RST, $Serial, $MyLocator, $MyQTH,
-- $CurrentOperator

-- are interpreted at transmission time

-- Should the callbacks add a space before or after the characters? Ideally not
-- but will this confuse novices.

We need a some kind of separator to allow us to parse things at all!

-- all messages will be converted to "UPPER CASE" before sending

CWMessages = {
    ["1"] = "CQ CQ CQ de $MyCall",
    ["2"] = "$MyCall",
    ["3"] = "$HisCall",
    ["4"] = "$CorrectedCall UR $RST $Serial at $MyLocator $MyQTH",
    }
SetMessages(CWMessages)

// WGV codings:

$1 mycall
$2 "^"
$3 "&"
$4 serial sent
$5 myloc
$6 file contents
$7 his call (autocorrect; only if not as originally sent)

// we will use???

*/
MORSEMSG::MORSEMSG() : repeat( false )
{}
MORSEMSG::~MORSEMSG()
{}
void MORSEMSG::setMessage( const QString &pvalue )
{
   msg = pvalue;
}

