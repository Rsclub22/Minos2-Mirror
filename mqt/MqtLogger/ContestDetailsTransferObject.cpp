#include "ContestDetailsTransferObject.h"

ContestDetailsTransferObject::ContestDetailsTransferObject()
{

}
void ContestDetailsTransferObject::validateLoc( )
{
    double ode, odn;
    if ( myloc.validate( ode, odn ) == LOC_OK )
    {
        locValid = true;
    }
    else
        locValid = false;

    Locator nloc;
    nloc.loc.setValue(myloc.loc.getValue().left(4) + "MM");
    if ( nloc.validate( ode, odn ) == LOC_OK )
    {
        locValid = true;
        myloc.valRes = LOC_OK;
    }
    else
        locValid = false;

}
void ContestDetailsTransferObject::initialiseINI()
{
   entryBundle.setProfile( BundleFile::bundleFiles[ epENTRYPROFILE ] );
   QTHBundle.setProfile( BundleFile::bundleFiles[ epQTHPROFILE ] );
   stationBundle.setProfile( BundleFile::bundleFiles[ epSTATIONPROFILE ] );
   entryBundleName.setInitialValue( entryBundle.getSection() );
   QTHBundleName.setInitialValue( QTHBundle.getSection() );
   stationBundleName.setInitialValue( stationBundle.getSection() );
}
void ContestDetailsTransferObject::setINIDetails()
{
   // extras that CAN come from INI file - implements bundle override
   if ( entryBundle.getSection() != entryBundle.noneBundle )
   {
      entryBundle.startGroup();
      entryBundle.getStringProfile( eepCall, mycall.fullCall );
      entryBundle.getStringProfile( eepEntrant, entrant );
      entryBundle.getStringProfile( eepMyName, entName );
      entryBundle.getStringProfile( eepMyCall, entCall );
      //   entryBundle.getStringProfile(eepSection, entSect);

      entryBundle.getStringProfile( eepMyAddress1, entAddr1 );
      entryBundle.getStringProfile( eepMyAddress2, entAddr2 );
      entryBundle.getStringProfile( eepMyCity, entCity );
      entryBundle.getStringProfile( eepMyPostCode, entPostCode );
      entryBundle.getStringProfile( eepMyCountry, entCountry );
      entryBundle.getStringProfile( eepMyPhone, entPhone );
      entryBundle.getStringProfile( eepMyEmail, entEMail );
      //   entryBundle.getStringProfile(eepBand, band);
      //   entryBundle.getStringProfile(eepSection, entSect);
      entryBundle.endGroup();
   }


   if ( QTHBundle.getSection() != QTHBundle.noneBundle )
   {
      QTHBundle.startGroup();
      QTHBundle.getStringProfile( eqpLocator, myloc.loc );

      if ( districtMult.getValue() )
         QTHBundle.getStringProfile( eqpDistrict, location );
      else
         if ( otherExchange.getValue() && location.getValue().size() == 0 )
            QTHBundle.getStringProfile( eqpLocation, location );

      QTHBundle.getStringProfile( eqpStationQTH1, sqth1 );
      QTHBundle.getStringProfile( eqpStationQTH2, sqth2 );
      QTHBundle.getStringProfile( eqpASL, entASL );
      QTHBundle.endGroup();
   }

   if ( stationBundle.getSection() != stationBundle.noneBundle )
   {
      stationBundle.startGroup();
      stationBundle.getStringProfile( espPower, power );
      stationBundle.getStringProfile( espTransmitter, entTx );
      stationBundle.getStringProfile( espReceiver, entRx );
      stationBundle.getStringProfile( espAntenna, entAnt );
      stationBundle.getStringProfile( espAGL, entAGL );
      stationBundle.getIntProfile(espOffset, bearingOffset);
      QString s;
      stationBundle.getStringProfile( espRadioName, s );
      radioName.setValue( PubSubName(s) );
      stationBundle.getStringProfile( espRotatorName, s );
      antennaName.setValue(PubSubName(s));
      stationBundle.endGroup();
   }
}
void ContestDetailsTransferObject::getFromContest(LoggerContestLog *ct)
{
    dateRange = ct->dateRange(DTGDISP);

    cfileName = ct->cfileName;
    minosFile = ct->minosFile;

    protectedContest = ct->protectedContest;

    name = ct->name;
    location = ct->location;

    mycall = ct->mycall;
    myloc = ct->myloc;

    allowLoc4 = ct->allowLoc4;
    allowLoc8 = ct->allowLoc8;

    RSTMandatoryField = ct->RSTMandatoryField;
    serialMandatoryField = ct->serialMandatoryField;
    locatorMandatoryField = ct->locatorMandatoryField;

    power = ct->power;
    currentMode = ct->currentMode;

    band = ct->band;
    otherExchange = ct->otherExchange;
    countryMult = ct->countryMult;
    nonGCountryMult = ct->nonGCountryMult;
    locMult = ct->locMult;
    GLocMult = ct->GLocMult;
    districtMult = ct->districtMult;

    M7Mults = ct->M7Mults;

    usesBonus = ct->usesBonus;
    bonusType = ct->bonusType;

    MGMContestRules = ct->MGMContestRules;

    powerWatts = ct->powerWatts;
    scoreMode = ct->scoreMode;
    DTGStart = ct->DTGStart;
    DTGEnd = ct->DTGEnd;

    bearingOffset = ct->bearingOffset;

    currentOp1 = ct->currentOp1;
    currentOp2 = ct->currentOp2;

    oplist = ct->oplist;

    opsQSO1 = ct->opsQSO1;
    opsQSO2 = ct->opsQSO2;

    locValid = ct->locValid;
    NonUKloc_mult = ct->NonUKloc_mult;
    UKloc_mult = ct->UKloc_mult;
    NonUKloc_multiplier = ct->NonUKloc_multiplier;
    UKloc_multiplier = ct->UKloc_multiplier;

    suppressProtected = ct->suppressProtected;
    unwriteable = ct->unwriteable;

    entryBundle = ct->entryBundle;
    entryBundleName = ct->entryBundleName;
    QTHBundle = ct->QTHBundle;
    QTHBundleName = ct->QTHBundleName;
    stationBundle = ct->stationBundle;
    stationBundleName = ct->stationBundleName;

    radioName = ct->radioName;
    antennaName = ct->antennaName;

    VHFContestName = ct->VHFContestName;

    entrant = ct->entrant;
    entSect = ct->entSect;
    sqth1 = ct->sqth1;
    sqth2 = ct->sqth2;
    entTx = ct->entTx;
    entRx = ct->entRx;
    entAnt = ct->entAnt;
    entAGL = ct->entAGL;
    entASL = ct->entASL;
    ops1 = ct->ops1;
    ops2 = ct->ops2;
    entCondx1 = ct->entCondx1;
    entCondx2 = ct->entCondx2;
    entCondx3 = ct->entCondx3;
    entCondx4 = ct->entCondx4;
    entName = ct->entName;
    entCall = ct->entCall;
    entAddr1 = ct->entAddr1;
    entAddr2 = ct->entAddr2;
    entPostCode = ct->entPostCode;
    entCity = ct->entCity;
    entCountry = ct->entCountry;
    entPhone = ct->entPhone;
    entEMail = ct->entEMail;
    sectionList = ct->sectionList;

    screenLayout = ct->screenLayout;

}
void ContestDetailsTransferObject::setToContest(LoggerContestLog *ct)
{
    ct->cfileName = cfileName;
    ct->minosFile = minosFile;

    ct->protectedContest = protectedContest;

    ct->name = name;
    ct->location = location;

    ct->mycall = mycall;
    ct->myloc = myloc;

    ct->allowLoc4 = allowLoc4;
    ct->allowLoc8 = allowLoc8;

    ct->RSTMandatoryField = RSTMandatoryField;
    ct->serialMandatoryField = serialMandatoryField;
    ct->locatorMandatoryField = locatorMandatoryField;

    ct->power = power;
    ct->currentMode = currentMode;

    ct->band = band;
    ct->otherExchange = otherExchange;
    ct->countryMult = countryMult;
    ct->nonGCountryMult = nonGCountryMult;
    ct->locMult = locMult;
    ct->GLocMult = GLocMult;
    ct->districtMult = districtMult;

    ct->M7Mults = M7Mults;

    ct->usesBonus = usesBonus;
    ct->bonusType = bonusType;

    ct->MGMContestRules = MGMContestRules;

    ct->powerWatts = powerWatts;
    ct->scoreMode = scoreMode;
    ct->DTGStart = DTGStart;
    ct->DTGEnd = DTGEnd;

    ct->bearingOffset = bearingOffset;

    ct->currentOp1 = currentOp1;
    ct->currentOp2 = currentOp2;

    ct->oplist = oplist;

    ct->opsQSO1 = opsQSO1;
    ct->opsQSO2 = opsQSO2;

    ct->locValid = locValid;
    ct->NonUKloc_mult = NonUKloc_mult;
    ct->UKloc_mult = UKloc_mult;
    ct->NonUKloc_multiplier = NonUKloc_multiplier;
    ct->UKloc_multiplier = UKloc_multiplier;

    ct->suppressProtected = suppressProtected;
    ct->unwriteable = unwriteable;

    ct->entryBundle = entryBundle;
    ct->entryBundleName = entryBundleName;
    ct->QTHBundle = QTHBundle;
    ct->QTHBundleName = QTHBundleName;
    ct->stationBundle = stationBundle;
    ct->stationBundleName = stationBundleName;

    ct->radioName = radioName;
    ct->antennaName = antennaName;

    ct->VHFContestName = VHFContestName;

    ct->entrant = entrant;
    ct->entSect = entSect;
    ct->sqth1 = sqth1;
    ct->sqth2 = sqth2;
    ct->entTx = entTx;
    ct->entRx = entRx;
    ct->entAnt = entAnt;
    ct->entAGL = entAGL;
    ct->entASL = entASL;
    ct->ops1 = ops1;
    ct->ops2 = ops2;
    ct->entCondx1 = entCondx1;
    ct->entCondx2 = entCondx2;
    ct->entCondx3 = entCondx3;
    ct->entCondx4 = entCondx4;
    ct->entName = entName;
    ct->entCall = entCall;
    ct->entAddr1 = entAddr1;
    ct->entAddr2 = entAddr2;
    ct->entPostCode = entPostCode;
    ct->entCity = entCity;
    ct->entCountry = entCountry;
    ct->entPhone = entPhone;
    ct->entEMail = entEMail;
    ct->sectionList = sectionList;

    ct->screenLayout = screenLayout;

}
