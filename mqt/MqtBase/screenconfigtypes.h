#ifndef SCREENCONFIGTYPES_H
#define SCREENCONFIGTYPES_H

#include <QObject>

enum SCType
{
    sctMainScreen,
    sctScreen,
    sctAux,
    sctLog,
    sctRigControl,
    sctRunButtons,
    sctBandSwitch,
    sctRotControl,
    sctSkyScanControl,
    sctRotCompassDisplay,
    sctRotPresets,
    sctRotSkyScanPresets,
    sctQSOEdit,
    sctNextQSODetails,
    sctThisMatch,
    sctOtherMatch,
    sctArchiveMatch,
    sctChat,
    sctCluster,
    sctWsjtx,
    sctBandmap,
    sctSplit,
    sctTxVmButtons,
    sctQrzDisplay,
    sctQsoMap,
    sctDMButtons,
    sctNone,

    sctkLogins,
    sctkCallList,
    sctkAirScout,
    sctkMessageList,
    sctkMeepList,
    sctkASActive,
    sctkSendMeep,
    sctkButtons
};

class ScreenConfigTypes
{
public:
    ScreenConfigTypes();
};

#endif // SCREENCONFIGTYPES_H
