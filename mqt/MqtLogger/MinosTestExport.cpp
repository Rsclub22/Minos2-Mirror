/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "LoggerContest.h"
#include "XMPPStanzas.h"
#include "contacts.h"
#include "MTrace.h"
#include "MinosParameters.h"
#include "MinosTestExport.h"

//==============================================================================
const char * MinosTestExport::fileHeader =
                         QT_TR_NOOP("====================================================\n"
                         "\nDO NOT SEND THIS FILE AS YOUR ENTRY!\n\n"
                         "Use \"File\" | \"Produce Entry/Export File...\"\n"
                         "Export as Reg1Test(entry)\n"
                         "and send the .EDI file produced.\n"
                         "====================================================\n")
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

   TIXML_STRING req = m->getActionMessage() + "\n";
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
   ct->setStanza( static_cast<unsigned int>(exp_stanzaCount), static_cast<int>(fpos), static_cast<int>(fpos) + written );


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

   ct->hfContest.addIfDirty(st, "hf", dirty);
   ct->appVersion.addIfDirty(st, "version", dirty);
   ct->name.addIfDirty( st, "name", dirty );
   ct->cabrilloName.addIfDirty(st, "cabrilloName", dirty);
   ct->calType.addIfDirty(st, "calType", dirty);
   ct->contestBands.addIfDirty( st, "band", dirty );
   ct->currentBand.addIfDirty( st, "currentBand", dirty );
   ct->bandsList.addIfDirty(st, "bandsList", dirty);
   ct->entSect.addIfDirty( st, "section", dirty );
   ct->sectionList.addIfDirty( st, "sectionList", dirty );
   if ( ct->scoreMode.isDirty() )
   {
      st->addMember( ( ct->scoreMode.getValue( dirty ) == PPKM ), "scoreKms" );
   }
   ct->DTGStart.addIfDirty( st, "startTime", dirty );
   ct->DTGEnd.addIfDirty( st, "endTime", dirty );
   ct->districtMult.addIfDirty( st, "districtMult", dirty );
   ct->districtBonus.addIfDirty( st, "districtBonus", dirty );
   ct->countryMult.addIfDirty( st, "DXCCMult", dirty );
   ct->locMult.addIfDirty( st, "locMult", dirty );
   ct->exchangeRequired.addIfDirty( st, "QTHReq", dirty );
   ct->exchangeDashAllowed.addIfDirty( st, "QTHOpt", dirty );
   ct->otherMult.addIfDirty(st, "OtherMultType", dirty);
   ct->asymmetricMult.addIfDirty(st, "AsymmetricMult", dirty);
   ct->allowLoc4.addIfDirty( st, "AllowLoc4", dirty );
   ct->allowLoc8.addIfDirty( st, "AllowLoc8", dirty );
   ct->currentMode.addIfDirty(st, "currentMode", dirty);
   ct->modeList.addIfDirty(st, "modeList", dirty);

   ct->RSTMandatoryField.addIfDirty(st, "RSTField", dirty);
   ct->serialMandatoryField.addIfDirty(st, "serialField", dirty);
   ct->locatorMandatoryField.addIfDirty(st, "locatorField", dirty);

   ct->usesBonus.addIfDirty( st, "UKACBonus", dirty );
   ct->bonusType.addIfDirty( st, "BonusType", dirty );
   ct->MGMContestRules.addIfDirty(st, "MGMContestRules", dirty);

   ct->bandPointsMultiplier.addIfDirty(st, "bandPointsMultiplier", dirty);

   ct->screenLayout.addIfDirty(st, "ScreenLayout", dirty);

   ct->currentFKeySet.addIfDirty(st, "currentFKeySet", dirty);
   ct->watchedADIFFile.addIfDirty(st, "WatchADIFFile", dirty);
   ct->watchedADIFLastOffset.addIfDirty(st, "WatchADIFLO", dirty);

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

void MinosTestExport::exportQSOMap( QSharedPointer<QFile> expfd )
{
    RPCParamStruct * st = new RPCParamStruct;
    makeHeader( st, 1 );

    bool dirty = false;
    ct->zoomLevel.addIfDirty( st, "zoom", dirty );
    ct->centreLat.addIfDirty( st, "lat", dirty );
    ct->centreLon.addIfDirty( st, "lon", dirty );

    if ( dirty )
    {
       sendRequest( expfd, "MinosQSOMap", st );
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
int MinosTestExport::exportQSO(QSharedPointer<QFile> expfd, const QSharedPointer<BaseContact> lct )
{
   RPCParamStruct * st = new RPCParamStruct;
   makeHeader( st, lct->getLogSequence() );

   bool dirty = false;
   if ( lct->timeOn.isDirty() )
   {
      st->addDtgMember( lct->timeOn.getIsoDTG( dirty ), "QSOStartTime" );
   }
   if ( lct->timeOff.isDirty() )
   {
      st->addDtgMember( lct->timeOff.getIsoDTG( dirty ), "logTime" );
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
   ct->bandsList.addIfDirty( st, "bandsList", dirty );
   lct->forcedMult.addIfDirty( st, "forcedMult", dirty );
   lct->sentExchange.addIfDirty( st, "sentExchange", dirty );
   lct->getFrequency().addIfDirty( st, "frequency", dirty );
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

        QVector<QSharedPointer<BandInfo> > bands;
        BandList::getBandList().loadAllBands(bands, false);
        for(const auto &b: QASCONST(bands))
        {
             QString bandIni = "bandFilter" + b->normalisedName();
             QString distIni = "distanceFilter" + b->normalisedName();
             QString igdistIni = "ignoreDistanceFlag" + b->normalisedName();
             QString igedistIni = "ignoreEmptyDistanceFlag" + b->normalisedName();

             st->addMember(clusterFilter.getValue().getBandFilter(b->name()), bandIni);
             st->addMember(clusterFilter.getValue().getDistanceFilter(b->name()), distIni);
             st->addMember(clusterFilter.getValue().getIgnoreDistanceFlag(b->name()), igdistIni);
             st->addMember(clusterFilter.getValue().getIgnoreEmptyDistanceFlag(b->name()), igedistIni);
        }

        st->addMember(clusterFilter.getValue().getModeFilter("NONE"), "modeFilterNONEMODE");
        st->addMember(clusterFilter.getValue().getModeFilter(hamlibData::CW), "modeFilterCW");
        st->addMember(clusterFilter.getValue().getModeFilter(hamlibData::LSB), "modeFilterLSBMODE");
        st->addMember(clusterFilter.getValue().getModeFilter(hamlibData::USB), "modeFilterUSBMODE");
        st->addMember(clusterFilter.getValue().getModeFilter(hamlibData::FM), "modeFilterFMMODE");
        st->addMember(clusterFilter.getValue().getModeFilter(hamlibData::RTTY), "modeFilterRTTYMODE");
        st->addMember(clusterFilter.getValue().getModeFilter("PSK31"), "modeFilterPSK31MODE");
        st->addMember(clusterFilter.getValue().getModeFilter("FT4"), "modeFilterFT4MODE");
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
        st->addMember(bandmapFilter.getValue().getModeFilter(hamlibData::CW), "modeFilterCW");
        st->addMember(bandmapFilter.getValue().getModeFilter(hamlibData::LSB), "modeFilterLSBMODE");
        st->addMember(bandmapFilter.getValue().getModeFilter(hamlibData::USB), "modeFilterUSBMODE");
        st->addMember(bandmapFilter.getValue().getModeFilter(hamlibData::FM), "modeFilterFMMODE");
        st->addMember(bandmapFilter.getValue().getModeFilter(hamlibData::RTTY), "modeFilterRTTYMODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("PSK31"), "modeFilterPSK31MODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("FT4"), "modeFilterFT4MODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("FT8"), "modeFilterFT8MODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("MSK144"), "modeFilterMSK144MODE");
        st->addMember(bandmapFilter.getValue().getModeFilter("JT65"), "modeFilterJT65MODE");
        st->addMember(bandmapFilter.getValue().getDistanceFilter(), "distanceFilter");
        st->addMember(bandmapFilter.getValue().getIgnoreDistanceFlag(), "ignoreDistanceFlag");
        st->addMember(bandmapFilter.getValue().getIgnoreEmptyDistanceFlag(), "ignoreEmptyDistanceFlag");
        sendRequest(expfd, "MinosBandmapFilter", st);

    }
}


void MinosTestExport::exportQSOMapFilter(QSharedPointer<QFile> expfd)
{
    MinosItem<BandmapClientFilterSettings> qsomapFilter = ct->QSOMapFilterSettings;
    if (qsomapFilter.isDirty())
    {
        RPCParamStruct * st = new RPCParamStruct;
        makeHeader( st, 1 );

        st->addMember(qsomapFilter.getValue().getModeFilter("NONE"), "modeFilterNONEMODE");
        st->addMember(qsomapFilter.getValue().getModeFilter(hamlibData::CW), "modeFilterCW");
        st->addMember(qsomapFilter.getValue().getModeFilter(hamlibData::LSB), "modeFilterLSBMODE");
        st->addMember(qsomapFilter.getValue().getModeFilter(hamlibData::USB), "modeFilterUSBMODE");
        st->addMember(qsomapFilter.getValue().getModeFilter(hamlibData::FM), "modeFilterFMMODE");
        st->addMember(qsomapFilter.getValue().getModeFilter(hamlibData::RTTY), "modeFilterRTTYMODE");
        st->addMember(qsomapFilter.getValue().getModeFilter("PSK31"), "modeFilterPSK31MODE");
        st->addMember(qsomapFilter.getValue().getModeFilter("FT4"), "modeFilterFT4MODE");
        st->addMember(qsomapFilter.getValue().getModeFilter("FT8"), "modeFilterFT8MODE");
        st->addMember(qsomapFilter.getValue().getModeFilter("MSK144"), "modeFilterMSK144MODE");
        st->addMember(qsomapFilter.getValue().getModeFilter("JT65"), "modeFilterJT65MODE");
        st->addMember(qsomapFilter.getValue().getDistanceFilter(), "distanceFilter");
        st->addMember(qsomapFilter.getValue().getIgnoreDistanceFlag(), "ignoreDistanceFlag");
        st->addMember(qsomapFilter.getValue().getIgnoreEmptyDistanceFlag(), "ignoreEmptyDistanceFlag");
        sendRequest(expfd, "MinosQSOMapFilter", st);

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
void MinosTestExport::exportRunMemory(QSharedPointer<QFile> expfd, MinosItem<memoryData::memData> mem )
{
    if (mem.isDirty())
    {
        RPCParamStruct * st = new RPCParamStruct;
        makeHeader( st, 1 );

        st->addMember(mem.getValue().memno, "memno");
        st->addMember(mem.getValue().freq.str(), "freq");
        st->addMember(mem.getValue().band, "band");
        st->addMember(mem.getValue().mode, "mode");

        sendRequest(expfd, "MinosRunMemory", st);
    }
}
void MinosTestExport::exportAllMemories(QSharedPointer<QFile> expfd )
{
    for(auto &m:ct->runMemories)
    {
        for (auto &r: m)
        {
            exportRunMemory( expfd, r);
        }
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
    ct->showWorkedCountries.addIfDirty( st, "sw", dirty );
    ct->showUnworkedCountries.addIfDirty( st, "su", dirty );
    ct->showWorkedDistricts.addIfDirty( st, "dw", dirty );
    ct->showUnworkedDistricts.addIfDirty( st, "du", dirty );

    bool siDirty = false;

    for (int i = 0; i < STACKITEMS; i++)
    {
         ct->currentStackItems[i].addIfDirty( st, "sitem" + QString::number(i), siDirty);
    }
    if (siDirty)
    {
        ct->currentStackItemsValid = true;
    }

    if ( dirty || siDirty )
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
void MinosTestExport::exportDetails(QSharedPointer<QFile> expfd )
{
    exportMode( expfd );
    exportContest( expfd );
    exportQTH( expfd );
    exportEntry( expfd );
    exportStation( expfd );
    exportCurrent( expfd );
    exportOperators( expfd );
    exportApps(expfd);
    exportBundles( expfd );
    exportAllMemories(expfd);
    exportStackDisplay(expfd);
    exportClusterFilter(expfd);
    exportBandmapFilter(expfd);
    exportQSOMap(expfd);
    exportQSOMapFilter(expfd);
}
int MinosTestExport::exportAllDetails(QSharedPointer<QFile> minosContestFile, bool newfile )
{
   if ( newfile )
   {
      //      QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' version='1.0'>" ;
      QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://minos.goodey.org.uk/streams' xmlns='minos:client' version='1.0'>" ;
      lbuff += "\n<!--\n" + tr(fileHeader) + "-->\n";

      writeFile(minosContestFile, lbuff);

   }

   exportDetails(minosContestFile);
   return exp_stanzaCount;
}

int MinosTestExport::exportTest( QSharedPointer<QFile> expfd, int mindump, int maxdump )
{
   exp_stanzaCount = 0;
   QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://minos.goodey.org.uk/streams' xmlns='minos:client' version='1.0'>" ;
   lbuff += "\n<!--\n" + tr(fileHeader) + "-->\n";
   writeFile(expfd, lbuff);

   // export a sequence of Minos stanzas

   exportDetails(expfd);

   bool inDump = false;
   for(auto const &dct: QASCONST(ct->ctList))
   {
       QSharedPointer<BaseContact> lct = dct.wt;

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

