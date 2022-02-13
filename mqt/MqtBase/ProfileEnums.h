/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef ProfileEnumsH
#define ProfileEnumsH
//----------------------------------------------------------------------------
enum LOGGERPROFILE {
   elpPreloadFile, elpPreloadSection,
   elpListsFile, elpListsSection,
   elpDisplayFile, elpDisplaySection,
   elpOperatorFile, elpOperatorSection,
   elpListDirectory, elpLogDirectory,
   elpLocsFile,
   elpEntryFile,
   elpStationFile,
   elpQTHFile,
    elpWSJTXUdpRecEnabled,
    elpWSJTXUdpRecPath,
   elpWSJTX1Enabled,
   elpWSJTX1Port,
   elpWSJTX1GroupAddress,
   elpWSJTX1AutoEnabled,
   elpWSJTX1TestEnabled,
    elpWSJTX1TestCQ,
    elpWSJTX1NonTestCQ,

    elpWSJTX2Enabled,
    elpWSJTX2Port,
    elpWSJTX2GroupAddress,
    elpWSJTX3Enabled,
    elpWSJTX3Port,
    elpWSJTX3GroupAddress,
    elpWSJTX4Enabled,
    elpWSJTX4Port,
    elpWSJTX4GroupAddress,


   elpcontactsSelect,
   elpextCSSelect,
   elpwsjtxRbSelect,
   elpADIFSelect,
   elpcontactsAddr,
   elpcontactsPort,
   elpextCSAddr,
   elpextCSPort,
   elpwsjtxRbAddr,
   elpwsjtxRbPort,
   elpADIFAddr,
   elpADIFPort,
   elpHelpFile, elpHelpEntryURL,
   elpPDFFile,
   elpReadabilityInit,
   elpAutoFill,
   elpShowOperateTime,
   elpTabforSandP,
   elpShowAuxHeaders,

   elpAgeProtectContests,
   elpAgeToProtectContests,
   elpBandmapOldStyle,
   elpAddBandMapTuningEnable,
   elpAddBandMapTuningTolerance,
   elpBandMapDisableNotShown,
   elpBandMapDisableLoggedCalls,
   elpBandMapDisablePlaceHolders,
   elpBandmapInvert,
   elpCQRit,

   elpContestStartIgnorePresetFreq,
   elpContestChangeIgnorePreviousFreq,
   elpContestChangeRestoreContestMode,

   elpContestTurnOffOperatingFreqColorRadioDial,
   elpBandMapTurnOffOperatingFreqStrip,
   elpBandMapFollowRadioModeOperatingFreqStrip,
   elpBandMapMouseInFrameDelay,
   elpShowDerivedLoc,

   elpBandMapTraceDebug,
   elpClusterTraceDebug


};
enum PRELOADPROFILE {eppSession, eppCurrent, eppDefSession};

enum DISPLAYPROFILE {edpShowContinentEU, edpShowContinentAS,
                     edpShowContinentAF, edpShowContinentOC,
                     edpShowContinentSA, edpShowContinentNA,
                     edpShowWorked, edpShowUnworked,
                     edpShowOperators,
                     edpEditor, edpStatisticsPeriod1, edpStatisticsPeriod2,
                     edpDefaultLayout,
                     edpProtectedLayout,
                     edpListCompression,
                     edpQSOFieldFont,
                     edpSeparateIcons,
                     edpls,
                     edpcml,
                     edpcmt,
                     edpcmr,
                     edpcmb
                    };
enum ENTRYPROFILE {eepCall,
                   eepMainOp, eepSecondOp,
                   eepEntrant, eepMyName, eepMyCall,
                   eepMyAddress1, eepMyAddress2, eepMyCity, eepMyPostCode, eepMyCountry,
                   eepMyPhone, eepMyEmail};
enum QTHPROFILE {eqpLocator, eqpDistrict, eqpLocation, eqpStationQTH1, eqpStationQTH2, eqpASL, eqpITUZone, eqpCQZone};
enum STATIONPROFILE {espPower, espTransmitter, espReceiver, espAntenna, espAGL, espOffset, espRadioName, espRotatorName};

enum PROFILES {epLOGGERPROFILE, epPRELOADPROFILE, epLISTSPROFILE, epDISPLAYPROFILE,
               epENTRYPROFILE, epQTHPROFILE, epSTATIONPROFILE, epLOCSQUARESPROFILE,
               epMAXPROFILE};

enum SHOWOPERATINGTIME {otNone, otRSGB, otIARU};
#endif
