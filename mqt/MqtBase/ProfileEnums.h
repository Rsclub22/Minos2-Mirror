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
   elpWSJTX1Enabled,
   elpWSJTX1Port,
   elpWSJTX1GroupAddress,
   elpWSJTX1AutoEnabled,
   elpWSJTX1DataPath,
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
   elpAutoFill,
   elpAllowHF,
   elpProgressDelay,
   elpAgeToProtectContests,
   elpBandmapOldStyle,
   elpAddBandMapTuningTolerance,


   elpDefaultFilterDistance_1_8MHz,
   elpDefaultFilterDistance_3_5MHz,
   elpDefaultFilterDistance_7MHz,
   elpDefaultFilterDistance_14MHz,
   elpDefaultFilterDistance_21MHz,
   elpDefaultFilterDistance_28MHz,

   elpDefaultFilterDistance_50MHz,
   elpDefaultFilterDistance_70MHz,
   elpDefaultFilterDistance_144MHz,
   elpDefaultFilterDistance_432MHz,
   elpDefaultFilterDistance_1296MHz,
   elpDefaultFilterDistance_2300MHz,
   elpDefaultFilterDistance_3_4GHz,
   elpDefaultFilterDistance_5_6GHz,
   elpDefaultFilterDistance_10GHz,

   elpBandmapStartZoomLevel_1_8MHz,
   elpBandmapStartZoomLevel_3_5MHz,
   elpBandmapStartZoomLevel_7MHz,
   elpBandmapStartZoomLevel_14MHz,
   elpBandmapStartZoomLevel_21MHz,
   elpBandmapStartZoomLevel_28MHz,


   elpBandmapStartZoomLevel_50MHz,
   elpBandmapStartZoomLevel_70MHz,
   elpBandmapStartZoomLevel_144MHz,
   elpBandmapStartZoomLevel_432MHz,
   elpBandmapStartZoomLevel_1296MHz,
   elpBandmapStartZoomLevel_2300MHz,
   elpBandmapStartZoomLevel_3_4GHz,
   elpBandmapStartZoomLevel_5_6GHz,
   elpBandmapStartZoomLevel_10GHz,

   elpContestStartIgnorePresetFreq,
   elpContestChangeIgnorePreviousFreq,
   elpContestChangeRestoreContestMode,

   elpBandMapTurnOffOperatingFreqStrip,
   elpBandMapFollowRadioModeOperatingFreqStrip,

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
                     edpCurrentLayout,
                     edpListCompression,
                     edpQSOFieldFont
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
#endif
