/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

#include "rotatorcommon.h"
#include "LoggerContest.h"
#include "contacts.h"
#include "MinosTestExport.h"

//==============================================================================
const char * MinosTestExport::fileHeader =
                         QT_TR_NOOP("====================================================\r\n"
                         "\r\nDO NOT SEND THIS FILE AS YOUR ENTRY!\r\n\r\n"
                         "Use \"File\" | \"Produce Entry/Export File...\"\r\n"
                         "Export as Reg1Test(entry)\r\n"
                         "and send the .EDI file produced.\r\n"
                         "====================================================\r\n")
                         ;
//==============================================================================
MinosTestExport::MinosTestExport( LoggerContestLog * const ct ) : ct( ct ),
      exp_stanzaCount( ct->getCtStanzaCount() )
{}
MinosTestExport::MinosTestExport( ) : ct( nullptr ), exp_stanzaCount( 0 )
{}
MinosTestExport::~MinosTestExport()
{}
// We need to be able to send stanzas remote - or do we do it from the disk copy?

// AND (as for import) we need to get the contest object to provide the stanzas

void MinosTestExport::sendRequest(QSharedPointer<QFile> expfd, const QString &cmd, RPCParamStruct *st )
{
   RPCArgs * MArgs = new RPCArgs;
   MArgs->addParam( QSharedPointer<RPCParam>(st) );

   RPCRequest *m = new RPCRequest( "", cmd );
   m->args = MArgs->args;

   TIXML_STRING req = m->getActionMessage() + "\r\n";
   trace(QString(req.c_str()));

   qint64 fpos = expfd->size();
   if (!expfd->seek(fpos))
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("(write) seek failed!") );
   }
   qint64 written = expfd->write( req.c_str(), req.size() );
   if ( written != static_cast<int>(req.size()) )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( tr("bad reply from write!") );
   }
   // set the stanza into the contest so we can monitor it later
   exp_stanzaCount += 1;
   ct->setStanza( static_cast<unsigned int>(exp_stanzaCount), static_cast<int>(fpos) );


   delete m;
   delete MArgs;
}
void MinosTestExport::makeHeader( RPCParamStruct * st, unsigned long ls )
{
   dtg tnow( true );
   st->addMember( static_cast< int>(ls), "lseq" );
   st->addDtgMember( tnow.getIsoDTG(), "uDTG" );
}
void MinosTestExport::exportMode(QSharedPointer<QFile> expfd )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;

   ct->getProtectedState().addIfDirty( st, "protectedContest", dirty );
   if ( dirty )
   {
      sendRequest( expfd, "MinosLogMode", st );
   }
   else
   {
      delete st;
   }
}
void MinosTestExport::exportContest( QSharedPointer<QFile> expfd )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;

   ct->appVersion.addIfDirty(st, "version", dirty);
   ct->name.addIfDirty( st, "name", dirty );
   ct->contestBands.addIfDirty( st, "band", dirty );
   ct->currentBand.addIfDirty( st, "currentBand", dirty );
   ct->entSect.addIfDirty( st, "section", dirty );
   ct->sectionList.addIfDirty( st, "sectionList", dirty );
   if ( ct->scoreMode.isDirty() )
   {
      st->addMember( ( ct->scoreMode.getValue( dirty ) == PPKM ), "scoreKms" );
   }
   ct->DTGStart.addIfDirty( st, "startTime", dirty );
   ct->DTGEnd.addIfDirty( st, "endTime", dirty );
   ct->districtMult.addIfDirty( st, "districtMult", dirty );
   ct->countryMult.addIfDirty( st, "DXCCMult", dirty );
   ct->nonGCountryMult.addIfDirty( st, "NonGCtryMult", dirty );
   ct->locMult.addIfDirty( st, "locMult", dirty );
   ct->GLocMult.addIfDirty( st, "GLocMult", dirty );
   ct->otherExchange.addIfDirty( st, "QTHReq", dirty );
   ct->otherOptionalExchange.addIfDirty( st, "QTHOpt", dirty );
   ct->allowLoc4.addIfDirty( st, "AllowLoc4", dirty );
   ct->allowLoc8.addIfDirty( st, "AllowLoc8", dirty );
   ct->currentMode.addIfDirty(st, "currentMode", dirty);

   ct->RSTMandatoryField.addIfDirty(st, "RSTField", dirty);
   ct->serialMandatoryField.addIfDirty(st, "serialField", dirty);
   ct->locatorMandatoryField.addIfDirty(st, "locatorField", dirty);

   ct->M7Mults.addIfDirty( st, "M7Mults", dirty );
   ct->usesBonus.addIfDirty( st, "UKACBonus", dirty );
   ct->bonusType.addIfDirty( st, "BonusType", dirty );
   ct->MGMContestRules.addIfDirty(st, "MGMContestRules", dirty);

   ct->bandPointsMultiplier.addIfDirty(st, "bandPointsMultiplier", dirty);

   ct->screenLayout.addIfDirty(st, "ScreenLayout", dirty);

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogContest", st );
   }
   else
   {
      delete st;
   }
}
void MinosTestExport::exportQTH(QSharedPointer<QFile> expfd )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;
   ct->myloc.addIfDirty( st, "locator", dirty );
   ct->location.addIfDirty( st, "district", dirty );  // We need to differentiate location from exchange!
   ct->location.addIfDirty( st, "location", dirty );
   ct->sqth1.addIfDirty( st, "stationQTH1", dirty );
   ct->sqth2.addIfDirty( st, "stationQTH2", dirty );
   ct->entASL.addIfDirty( st, "ASL", dirty );

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogQTH", st );
   }
   else
   {
      delete st;
   }
}
void MinosTestExport::exportEntry( QSharedPointer<QFile> expfd )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;
   ct->mycall.addIfDirty( st, "call", dirty );
   ct->entrant.addIfDirty( st, "entrant", dirty );
   ct->entName.addIfDirty( st, "myName", dirty );
   ct->entCall.addIfDirty( st, "myCall", dirty );
   ct->entAddr1.addIfDirty( st, "myAddress1", dirty );
   ct->entAddr2.addIfDirty( st, "myAddress2", dirty );
   ct->entCity.addIfDirty( st, "myCity", dirty );
   ct->entCountry.addIfDirty( st, "myCountry", dirty );
   ct->entPostCode.addIfDirty( st, "myPostCode", dirty );
   ct->entPhone.addIfDirty( st, "myPhone", dirty );
   ct->entEMail.addIfDirty( st, "myEmail", dirty );

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogEntry", st );
   }
   else
   {
      delete st;
   }
}
void MinosTestExport::exportStation(QSharedPointer<QFile> expfd )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;
   ct->power.addIfDirty( st, "power", dirty );
   ct->entTx.addIfDirty( st, "transmitter", dirty );
   ct->entRx.addIfDirty( st, "receiver", dirty );
   ct->entAnt.addIfDirty( st, "antenna", dirty );
   ct->entAGL.addIfDirty( st, "AGL", dirty );
   ct->bearingOffset.addIfDirty(st, "offset", dirty );
   if (ct->radioName.isDirty())
   {
        st->addMember(ct->radioName.getValue().toString(), "radioName");
        dirty = true;
   }
   if (ct->antennaName.isDirty())
   {
        st->addMember( ct->antennaName.getValue().toString(), "rotatorName");
        dirty = true;
   }

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogStation", st );
   }
   else
   {
      delete st;
   }
}
void MinosTestExport::exportCurrent( QSharedPointer<QFile> expfd )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;
   ct->entCondx1.addIfDirty( st, "condx1", dirty );
   ct->entCondx2.addIfDirty( st, "condx2", dirty );
   ct->entCondx3.addIfDirty( st, "condx3", dirty );
   ct->entCondx4.addIfDirty( st, "condx4", dirty );

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogCurrent", st );
   }
   else
   {
      delete st;
   }
}
void MinosTestExport::exportOperators(QSharedPointer<QFile> expfd )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;

   ct->ops1.addIfDirty( st, "ops1", dirty );
   ct->ops2.addIfDirty( st, "ops2", dirty );
   ct->currentOp1.addIfDirty( st, "currentOp1", dirty );
   ct->currentOp2.addIfDirty( st, "currentOp2", dirty );

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogOperators", st );
   }
   else
   {
      delete st;
   }

}
void MinosTestExport::exportApps(QSharedPointer<QFile> expfd)
{
    RPCParamStruct * st = new RPCParamStruct;
    makeHeader( st, 1 );

    bool dirty = false;

    if ( dirty )
    {
       sendRequest( expfd, "MinosApps", st );
    }
    else
    {
       delete st;
    }
}

void MinosTestExport::exportBundles( QSharedPointer<QFile> expfd )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;

   ct->entryBundleName.addIfDirty( st, "entryBundle", dirty );
   ct->QTHBundleName.addIfDirty( st, "QTHBundle", dirty );
   ct->stationBundleName.addIfDirty( st, "stationBundle", dirty );
   ct->VHFContestName.addIfDirty(st, "VHFContestName", dirty );

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogBundles", st );
   }
   else
   {
      delete st;
   }

}
void MinosTestExport::exportComment(QSharedPointer<QFile> expfd, const QSharedPointer<BaseContact> lct )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, 1 );

   bool dirty = false;
   if ( lct->time.isDirty() )
   {
      st->addDtgMember( lct->time.getIsoDTG( dirty ), "logTime" );
   }
   if ( lct->contactFlags.isDirty() )
   {
      st->addMember( bool( (lct->contactFlags.getValue( dirty ) & LOCAL_COMMENT) != 0 ), "LocalComment" );
      st->addMember( bool( (lct->contactFlags .getValue( dirty ) & DONT_PRINT) != 0 ), "dontPrint" );
   }
   lct->comments.addIfDirty( st, "comment", dirty );

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogComment", st );
   }
   else
   {
      delete st;
   }
}
int MinosTestExport::exportQSO(QSharedPointer<QFile> expfd, const QSharedPointer<BaseContact> lct )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, lct->getLogSequence() );

   bool dirty = false;
   if ( lct->time.isDirty() )
   {
      st->addDtgMember( lct->time.getIsoDTG( dirty ), "logTime" );
   }
   if ( lct->contactFlags.isDirty() )
   {
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & VALID_DISTRICT ) != 0 ), "validDistrict" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & COUNTRY_FORCED ) != 0 ), "countryForced" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & UNKNOWN_COUNTRY ) != 0 ), "unknownCountry" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & NON_SCORING ) != 0 ), "nonScoring" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & MANUAL_SCORE ) != 0 ), "manualScore" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & DONT_PRINT ) != 0 ), "dontPrint" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & VALID_DUPLICATE ) != 0 ), "validDuplicate" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & TO_BE_ENTERED ) != 0 ), "toBeEntered" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & XBAND ) != 0 ), "xBand" );
      st->addMember( bool( ( lct->contactFlags.getValue( dirty ) & FORCE_LOG ) != 0 ), "Forced" );
   }
   trace(QString("export QSO CS %1 dirty %2 Loc %3 dirty %4")
         .arg(lct->cs.getFullCall()).arg(lct->cs.isDirty())
         .arg(lct->loc.getLoc()).arg(lct->loc.isDirty())
         );
   lct->cs.addIfDirty( st, "callRx", dirty );
   lct->reps.addIfDirty( st, "rstTx", dirty );
   lct->serials.addIfDirty( st, "serialTx", dirty );
   //   lct->extraText.addIfDirty(st, "exchangeTx", dirty);
   lct->repr.addIfDirty( st, "rstRx", dirty );
   lct->serialr.addIfDirty( st, "serialRx", dirty );
   lct->loc.addIfDirty( st, "locRx", dirty );
   lct->extraText.addIfDirty( st, "exchangeRx", dirty );
   lct->extraText.addIfDirty( st, "qthRx", dirty );
   lct->comments.addIfDirty( st, "commentsTx", dirty );
   lct->comments.addIfDirty( st, "commentsRx", dirty );
   lct->mode.addIfDirty( st, "modeTx", dirty );
   lct->mode.addIfDirty( st, "modeRx", dirty );
   lct->mgmSubmode.addIfDirty(st, "mgmSubmode", dirty);
   ct->power.addIfDirty( st, "power", dirty );
   ct->contestBands.addIfDirty( st, "band", dirty );
   ct->currentBand.addIfDirty( st, "currentBand", dirty );
   lct->forcedMult.addIfDirty( st, "forcedMult", dirty );
   lct->frequency.addIfDirty( st, "frequency", dirty );
   lct->rotatorHeading.addIfDirty( st, "rotatorHeading", dirty );
   lct->rigName.addIfDirty( st, "rigName", dirty );
   lct->contactScore.addIfDirty( st, "claimedScore", dirty );
   lct->op1.addIfDirty( st, "op1", dirty );
   lct->op2.addIfDirty( st, "op2", dirty );
   lct->cqResponse.addIfDirty(st, "cqResponse", dirty);

   if ( dirty )
   {
      sendRequest( expfd, "MinosLogQSO", st );
   }
   else
   {
      delete st;
   }
   return exp_stanzaCount;
}
void MinosTestExport::exportClusterFilter(QSharedPointer<QFile> expfd)
{
    MinosItem<ClusterClientFilterSettings> clusterFilter = ct->clusterFilterSettings;
    if (clusterFilter.isDirty())
    {
        RPCParamStruct * st = new RPCParamStruct;
        makeHeader( st, 1 );

        st->addMember(clusterFilter.getValue().callsignFilterList, "callsignList");
        st->addMember(clusterFilter.getValue().locatorFilterList, "locatorList");

        st->addMember(clusterFilter.getValue().getBandFilter("1.8 MHz"), "bandFilter1_8Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("1.8 MHz"), "distanceFilter1_8MHz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("1.8 MHz"), "ignoreDistanceFlag1_8MHz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("1.8 MHz"), "ignoreEmptyDistanceFlag1_8MHz");

        st->addMember(clusterFilter.getValue().getBandFilter("3.5 MHz"), "bandFilter3_5Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("3.5 MHz"), "distanceFilter3_5Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("3.5 MHz"), "ignoreDistanceFlag3_5Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("3.5 MHz"), "ignoreEmptyDistanceFlag3_5Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("7 MHz"), "bandFilter7Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("7 MHz"), "distanceFilter7Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("7 MHz"), "ignoreDistanceFlag7Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("7 MHz"), "ignoreEmptyDistanceFlag7Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("14 MHz"), "bandFilter14Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("14 MHz"), "distanceFilter14Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("14 MHz"), "ignoreDistanceFlag14Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("14 MHz"), "ignoreEmptyDistanceFlag14Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("21 MHz"), "bandFilter21Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("21 MHz"), "distanceFilter21Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("21 MHz"), "ignoreDistanceFlag21Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("21 MHz"), "ignoreEmptyDistanceFlag21Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("28 MHz"), "bandFilter28Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("28 MHz"), "distanceFilter28Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("28 MHz"), "ignoreDistanceFlag28Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("28 MHz"), "ignoreEmptyDistanceFlag28Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("50 MHz"), "bandFilter50Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("50 MHz"), "distanceFilter50Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("50 MHz"), "ignoreDistanceFlag50Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("50 MHz"), "ignoreEmptyDistanceFlag50Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("70 MHz"), "bandFilter70Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("70 MHz"), "distanceFilter70Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("70 MHz"), "ignoreDistanceFlag70Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("70 MHz"), "ignoreEmptyDistanceFlag70Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("144 MHz"), "bandFilter144Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("144 MHz"), "distanceFilter144Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("144 MHz"), "ignoreDistanceFlag144Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("144 MHz"), "ignoreEmptyDistanceFlag144Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("432 MHz"), "bandFilter432Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("432 MHz"), "distanceFilter432Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("432 MHz"), "ignoreDistanceFlag432Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("432 MHz"), "ignoreEmptyDistanceFlag432Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("1296 MHz"), "bandFilter1296Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("1296 MHz"), "distanceFilter1296Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("1296 MHz"), "ignoreDistanceFlag1296Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("1296 MHz"), "ignoreEmptyDistanceFlag1296Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("2300 MHz"), "bandFilter2300Mhz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("2300 MHz"), "distanceFilter2300Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("2300 MHz"), "ignoreDistanceFlag2300Mhz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("2300 MHz"), "ignoreEmptyDistanceFlag2300Mhz");

        st->addMember(clusterFilter.getValue().getBandFilter("3.4 GHz"), "bandFilter3_4Ghz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("3.4 GHz"), "distanceFilter3_4Ghz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("3.4 GHz"), "ignoreDistanceFlag3_4Ghz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("3.4 GHz"), "ignoreEmptyDistanceFlag3_4Ghz");

        st->addMember(clusterFilter.getValue().getBandFilter("5.6 GHz"), "bandFilter5_6Ghz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("5.6 GHz"), "distanceFilter5_6Ghz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("5.6 GHz"), "ignoreDistanceFlag5_6Ghz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("5.6 GHz"), "ignoreEmptyDistanceFlag5_6Ghz");

        st->addMember(clusterFilter.getValue().getBandFilter("10 GHz"), "bandFilter10Ghz");
        st->addMember(clusterFilter.getValue().getDistanceFilter("10 GHz"), "distanceFilter10Ghz");
        st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag("10 GHz"), "ignoreDistanceFlag10Ghz");
        st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag("10 GHz"), "ignoreEmptyDistanceFlag10Ghz");

        st->addMember(clusterFilter.getValue().getModeFilter("NONE"), "modeFilterNONE");
        st->addMember(clusterFilter.getValue().getModeFilter("CW"), "modeFilterCW");
        st->addMember(clusterFilter.getValue().getModeFilter("USB"), "modeFilterUSBMODE");
        st->addMember(clusterFilter.getValue().getModeFilter("FM"), "modeFilterFMMODE");
        st->addMember(clusterFilter.getValue().getModeFilter("RTTY"), "modeFilterRTTYMODE");
        st->addMember(clusterFilter.getValue().getModeFilter("PSK31"), "modeFilterPSK31MODE");
        st->addMember(clusterFilter.getValue().getModeFilter("FT8"), "modeFilterFT8MODE");
        st->addMember(clusterFilter.getValue().getModeFilter("MSK144"), "modeFilterMSK144MODE");
        st->addMember(clusterFilter.getValue().getModeFilter("JT65"), "modeFilterJT65MODE");


        sendRequest(expfd, "MinosClusterFilter", st);

    }
}

void MinosTestExport::exportBandmapFilter(QSharedPointer<QFile> expfd)
{
    MinosItem<BandmapClientFilterSettings> bandmapFilter = ct->bandmapFilterSettings;
    if (bandmapFilter.isDirty())
    {
        RPCParamStruct * st = new RPCParamStruct;
        makeHeader( st, 1 );

        st->addMember(bandmapFilter.getValue().getModeFilter("NONE"), "modeFilterNONEMODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("CW"), "modeFilterCW");
        st->addMember(bandmapFilter.getValue().getModeFilter("USB"), "modeFilterUSBMODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("FM"), "modeFilterFMMODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("RTTY"), "modeFilterRTTYMODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("PSK31"), "modeFilterPSK31MODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("FT8"), "modeFilterFT8MODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("MSK144"), "modeFilterMSK144MODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("JT65"), "modeFilterJT65MODE");
        st->addMember(bandmapFilter.getValue().getDistanceFilter(), "distanceFilter");
        st->addMember(bandmapFilter.getValue().getIgnoreDistanceFlag(), "ignoreDistanceFlag");
        st->addMember(bandmapFilter.getValue().getIgnoreEmptyDistanceFlag(), "ignoreEmptyDistanceFlag");
        sendRequest(expfd, "MinosBandmapFilter", st);

    }
}




void MinosTestExport::exportRigMemory(QSharedPointer<QFile> expfd, int memno )
{
    MinosItem<memoryData::memData> mem = ct->rigMemories[memno];
    if (mem.isDirty())
    {
        RPCParamStruct * st = new RPCParamStruct;
        makeHeader( st, 1 );

        st->addMember(memno, "memno");
        st->addMember(mem.getValue().callsign, "callsign");
        st->addMember(mem.getValue().freq.str(), "freq");
        st->addMember(mem.getValue().mode, "mode");
        st->addMember(mem.getValue().locator, "locator");
        st->addMember(mem.getValue().bearing, "bearing");
        st->addMember(mem.getValue().time, "time");
        st->addMember(mem.getValue().worked, "worked");

        sendRequest(expfd, "MinosRigMemory", st);
    }
}
void MinosTestExport::exportRunMemory(QSharedPointer<QFile> expfd, int memno )
{
    MinosItem<memoryData::memData> mem = ct->runMemories[memno];
    if (mem.isDirty())
    {
        RPCParamStruct * st = new RPCParamStruct;
        makeHeader( st, 1 );

        st->addMember(memno, "memno");
        st->addMember(mem.getValue().freq.str(), "freq");
        st->addMember(mem.getValue().mode, "mode");

        sendRequest(expfd, "MinosRunMemory", st);
    }
}
void MinosTestExport::exportAllMemories(QSharedPointer<QFile> expfd )
{
    for (int i = 0; i < ct->runMemories.size(); i++)
    {
        exportRunMemory( expfd, i);
    }
    for (int i = 0; i < ct->rigMemories.size(); i++)
    {
        exportRigMemory( expfd, i);
    }
}
int MinosTestExport::exportStackDisplay(QSharedPointer<QFile> expfd )
{
    RPCParamStruct * st = new RPCParamStruct;
    makeHeader( st, 1 );

    bool dirty = false;

    ct->statsPeriod1.addIfDirty( st, "sp1", dirty );
    ct->statsPeriod2.addIfDirty( st, "sp2", dirty );
    ct->showContinentEU.addIfDirty( st, "eu", dirty );
    ct->showContinentAS.addIfDirty( st, "as", dirty );
    ct->showContinentAF.addIfDirty( st, "af", dirty );
    ct->showContinentOC.addIfDirty( st, "oc", dirty );
    ct->showContinentSA.addIfDirty( st, "sa", dirty );
    ct->showContinentNA.addIfDirty( st, "na", dirty );
    ct->showWorked.addIfDirty( st, "sw", dirty );
    ct->showUnworked.addIfDirty( st, "su", dirty );

    for (int i = 0; i < STACKITEMS; i++)
    {
         ct->currentStackItems[i].addIfDirty( st, "sitem" + QString::number(i), dirty);
    }

    if ( dirty )
    {
       sendRequest( expfd, "MinosStackParams", st );
    }
    else
    {
       delete st;
    }
    return exp_stanzaCount;
}
void MinosTestExport::writeFile(QSharedPointer<QFile> minosContestFile, QString lbuff)
{
    QByteArray bbuff = lbuff.toUtf8();
    const char *cbuff = bbuff.data();
    int cblen = static_cast<int>(strlen(cbuff));
    qint64 ret = minosContestFile->write(cbuff, cblen);
    if ( ret != cblen )
    {
       MinosParameters::getMinosParameters() ->mshowMessage( tr("bad reply from write!") );
    }

}
int MinosTestExport::exportAllDetails(QSharedPointer<QFile> minosContestFile, bool newfile )
{
   if ( newfile )
   {
      //      QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' version='1.0'>" ;
      QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://minos.goodey.org.uk/streams' xmlns='minos:client' version='1.0'>" ;
      lbuff += "\r\n<!--\r\n" + tr(fileHeader) + "-->\r\n";

      writeFile(minosContestFile, lbuff);

   }
   exportMode( minosContestFile );
   exportContest( minosContestFile );
   exportQTH( minosContestFile );
   exportEntry( minosContestFile );
   exportStation( minosContestFile );
   exportCurrent( minosContestFile );
   exportOperators( minosContestFile );
   exportApps(minosContestFile);
   exportBundles( minosContestFile );
   exportAllMemories(minosContestFile);
   exportStackDisplay(minosContestFile);
   exportClusterFilter(minosContestFile);
   exportBandmapFilter(minosContestFile);

   return exp_stanzaCount;
}
// AND we need an export operator change
int MinosTestExport::exportTest( QSharedPointer<QFile> expfd, int mindump, int maxdump )
{
   exp_stanzaCount = 0;
   QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://minos.goodey.org.uk/streams' xmlns='minos:client' version='1.0'>" ;
   lbuff += "\r\n<!--\r\n" + tr(fileHeader) + "-->\r\n";
   writeFile(expfd, lbuff);

   // export a sequence of Minos stanzas

   exportMode( expfd );
   exportContest( expfd );
   exportQTH( expfd );
   exportEntry( expfd );
   exportStation( expfd );
   exportCurrent( expfd );
   exportOperators( expfd );     // not right... we need to log op changes
   exportApps(expfd);
   exportBundles( expfd );
   exportAllMemories(expfd);
   exportClusterFilter(expfd);

   bool inDump = false;
   for(auto const &dct: qAsConst(ct->ctList))
   {
       QSharedPointer<BaseContact> lct = dct.wt;

      if ( inDump && lct->contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY ) )
      {
         exportComment( expfd, lct );
         continue;
      }

      int serials = lct->serials.getValue().toInt();
      // dump the contact, until serial seen

      if ( ( serials >= mindump ) || ( mindump == 0 ) )
         inDump = true;

      if ( inDump && ( serials <= maxdump ) )
      {
         exportQSO( expfd, lct );
      }

      // Cannot assume that all serials are in order, so we need to go through
      // the whole contest
   }

   return exp_stanzaCount;
}
//=============================================================================

