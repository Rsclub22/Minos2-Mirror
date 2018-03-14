/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "logger_pch.h"

#include "MinosTestExport.h"

//==============================================================================
QString fileHeader = "<!--\r\n"
                         "====================================================\r\n"
                         "\r\nDO NOT SEND THIS FILE AS YOUR ENTRY!\r\n\r\n"
                         "Use \"File\" | \"Produce Entry/Export File...\"\r\n"
                         "Export as Reg1Test(entry)\r\n"
                         "and send the .EDI file produced.\r\n"
                         "====================================================\r\n"
                         "-->\r\n";
//==============================================================================
MinosTestExport::MinosTestExport( LoggerContestLog * const ct ) : ct( ct ),
      exp_stanzaCount( ct->getCtStanzaCount() )
{}
MinosTestExport::MinosTestExport( ) : ct( 0 ), exp_stanzaCount( 0 )
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

   int fpos = expfd->size();
   if (!expfd->seek(fpos))
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "(write) seek failed!" );
   }
   int written = expfd->write( req.c_str(), req.size() );
   if ( written != static_cast<int>(req.size()) )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "bad reply from write!" );
   }
   // set the stanza into the contest so we can monitor it later
   exp_stanzaCount += 1;
   ct->setStanza( exp_stanzaCount, fpos );


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

   ct->protectedContest.addIfDirty( st, "protectedContest", dirty );
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

   ct->name.addIfDirty( st, "name", dirty );
   ct->band.addIfDirty( st, "band", dirty );
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
   ct->allowLoc4.addIfDirty( st, "AllowLoc4", dirty );
   ct->allowLoc8.addIfDirty( st, "AllowLoc8", dirty );
   ct->currentMode.addIfDirty(st, "currentMode", dirty);

   ct->RSTField.addIfDirty(st, "RSTField", dirty);
   ct->serialField.addIfDirty(st, "serialField", dirty);
   ct->locatorField.addIfDirty(st, "locatorField", dirty);

   ct->M7Mults.addIfDirty( st, "M7Mults", dirty );
   ct->usesBonus.addIfDirty( st, "UKACBonus", dirty );
   ct->bonusType.addIfDirty( st, "BonusType", dirty );
   ct->MGMContestRules.addIfDirty(st, "MGMContestRules", dirty);

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
   ct->myloc.loc.addIfDirty( st, "locator", dirty );
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
   ct->mycall.fullCall.addIfDirty( st, "call", dirty );
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
   ct->radioName.addIfDirty(st, "radioName", dirty);
   ct->rotatorName.addIfDirty(st, "rotatorName", dirty);

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

    // This wants "useBundles"(?) and the bundle names selected
    bool dirty = false;

    ct->appRigControl.addIfDirty( st, "appRigControl", dirty );
    ct->appBandMap.addIfDirty( st, "appBandMap", dirty );
    ct->appRotator.addIfDirty( st, "appRotator", dirty );
    ct->appVoiceKeyer.addIfDirty( st, "appVoiceKeyer", dirty );

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

   // This wants "useBundles"(?) and the bundle names selected
   bool dirty = false;

   ct->entryBundleName.addIfDirty( st, "entryBundle", dirty );
   ct->QTHBundleName.addIfDirty( st, "QTHBundle", dirty );
   ct->stationBundleName.addIfDirty( st, "stationBundle", dirty );
   ct->appBundleName.addIfDirty( st, "appBundle", dirty );
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
   lct->cs.fullCall.addIfDirty( st, "callRx", dirty );
   lct->reps.addIfDirty( st, "rstTx", dirty );
   lct->serials.addIfDirty( st, "serialTx", dirty );
   //   lct->extraText.addIfDirty(st, "exchangeTx", dirty);
   lct->repr.addIfDirty( st, "rstRx", dirty );
   lct->serialr.addIfDirty( st, "serialRx", dirty );
   lct->loc.loc.addIfDirty( st, "locRx", dirty );
   lct->extraText.addIfDirty( st, "exchangeRx", dirty );
   lct->extraText.addIfDirty( st, "qthRx", dirty );
   lct->comments.addIfDirty( st, "commentsTx", dirty );
   lct->comments.addIfDirty( st, "commentsRx", dirty );
   lct->mode.addIfDirty( st, "modeTx", dirty );
   lct->mode.addIfDirty( st, "modeRx", dirty );
   ct->power.addIfDirty( st, "power", dirty );
   ct->band.addIfDirty( st, "band", dirty );
   lct->forcedMult.addIfDirty( st, "forcedMult", dirty );
   lct->frequency.addIfDirty( st, "frequency", dirty );
   lct->rotatorHeading.addIfDirty( st, "rotatorHeading", dirty );
   lct->rigName.addIfDirty( st, "rigName", dirty );
   lct->contactScore.addIfDirty( st, "claimedScore", dirty );
   lct->op1.addIfDirty( st, "op1", dirty );
   lct->op2.addIfDirty( st, "op2", dirty );

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
void MinosTestExport::exportRigMemory(QSharedPointer<QFile> expfd, int memno )
{
    MinosItem<memoryData::memData> mem = ct->rigMemories[memno];
    if (mem.isDirty())
    {
        RPCParamStruct * st = new RPCParamStruct;
        makeHeader( st, 1 );

        st->addMember(memno, "memno");
        st->addMember(mem.getValue().callsign, "callsign");
        st->addMember(mem.getValue().freq, "freq");
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
        st->addMember(mem.getValue().freq, "freq");
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
    ct->currentStackItem.addIfDirty( st, "sitem", dirty );
    ct->currentStack1Item.addIfDirty( st, "sitem1", dirty );
    ct->currentStack2Item.addIfDirty( st, "sitem2", dirty );
    ct->currentStack3Item.addIfDirty( st, "sitem3", dirty );

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
int MinosTestExport::exportAllDetails(QSharedPointer<QFile> minosContestFile, bool newfile )
{
   if ( newfile )
   {
      //      QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' version='1.0'>" ;
      QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://minos.goodey.org.uk/streams' xmlns='minos:client' version='1.0'>" ;
      lbuff += "\r\n" + fileHeader;
      int ret = minosContestFile->write(lbuff.toStdString().c_str(), lbuff.length());
      if ( ret != lbuff.length() )
      {
         MinosParameters::getMinosParameters() ->mshowMessage( "bad reply from write!" );
      }
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

   return exp_stanzaCount;
}
// AND we need an export operator change
int MinosTestExport::exportTest( QSharedPointer<QFile> expfd, int mindump, int maxdump )
{
   exp_stanzaCount = 0;
   QString lbuff = "<?xml version='1.0'?><stream:stream xmlns:stream='http://minos.goodey.org.uk/streams' xmlns='minos:client' version='1.0'>" ;
   lbuff += "\r\n" + fileHeader;
   int ret = expfd->write(lbuff.toStdString().c_str(), lbuff.length());
   if (  ret != lbuff.length() )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "bad reply from write!" );
   }

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

   bool inDump = false;
   foreach(MapWrapper<BaseContact> dct, ct->ctList)
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

