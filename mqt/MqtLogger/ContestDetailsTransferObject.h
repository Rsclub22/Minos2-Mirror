#ifndef CONTESTDETAILSTRANSFEROBJECT_H
#define CONTESTDETAILSTRANSFEROBJECT_H
#include "base_pch.h"
#include "LoggerContest.h"

class ContestDetailsTransferObject
{
    friend class ContestDetails;
    friend class TEntryOptionsForm;

public:
    ContestDetailsTransferObject();
    void getFromContest(LoggerContestLog *ct);
    void setToContest(LoggerContestLog *ct);
    void validateLoc();
    bool isMinosFile()
    {
       return minosFile;
    }
    bool isReadOnly( )
    {
       return (protectedContest.getValue() && ! suppressProtected) || unwriteable;
    }
    bool isProtected( )
    {
       return protectedContest.getValue();
    }
    void setProtected( bool s  )
    {
       if (protectedContest.getValue() && !s)
       {
          suppressProtected = true;
       }
       else if (s)
       {
          suppressProtected = false;
          protectedContest.setValue( true );
       }
    }
    bool isProtectedSuppressed( )
    {
       return suppressProtected;
    }
    void setProtectedSuppressed( bool s)
    {
       suppressProtected = s;
    }
    void initialiseINI();
    void setINIDetails();
private:
    QString cfileName;
    bool minosFile;

    MinosStringItem<QString> appVersion;
    MinosItem<bool> protectedContest;
    MinosStringItem<QString> name;         // name of contest
    MinosStringItem<QString> location;

    QString dateRange;

    Callsign mycall;  // CONTAINS MinosItem
    Locator myloc;  // CONTAINS MinosItem

    MinosItem<bool> allowLoc4;
    MinosItem<bool> allowLoc8;

    MinosItem<bool> RSTMandatoryField;
    MinosItem<bool> serialMandatoryField;
    MinosItem<bool> locatorMandatoryField;

    MinosStringItem<QString> power;
    MinosStringItem<QString> currentMode;

    MinosStringItem<QString> contestBands;
    MinosStringItem<QString> currentBand;
    MinosItem<bool> otherExchange;
    MinosItem<bool> otherOptionalExchange;
    MinosItem<bool> countryMult;
    MinosItem<bool> nonGCountryMult;
    MinosItem<bool> locMult;
    MinosItem<bool> GLocMult;
    MinosItem<bool> districtMult;

    MinosItem<bool> M7Mults;

    MinosItem<bool> usesBonus;
    MinosStringItem<QString> bonusType;

    MinosItem<bool> MGMContestRules;

    MinosItem<int> bandPointsMultiplier;

    MinosItem<bool> powerWatts;
    MinosItem<int> scoreMode;
    MinosStringItem<QString> DTGStart;
    MinosStringItem<QString> DTGEnd;

    MinosItem<int> bearingOffset;

    MinosStringItem<QString> currentOp1;         // current main op - derived from contacts
    MinosStringItem<QString> currentOp2;         // current second op - derived from contacts

    OperatorList oplist;

    QString opsQSO1;
    QString opsQSO2;

    bool locValid = false;
    bool NonUKloc_mult = false;
    bool UKloc_mult = false;
    int NonUKloc_multiplier = 0;
    int UKloc_multiplier = 0;

    bool suppressProtected = false;
    bool unwriteable = false;

    SettingsBundle entryBundle;      // individual LoggerContestLog
    MinosStringItem<QString> entryBundleName;
    SettingsBundle QTHBundle;        // individual QTH
    MinosStringItem<QString> QTHBundleName;
    SettingsBundle stationBundle;    // individual station
    MinosStringItem<QString> stationBundleName;

    MinosItem<PubSubName> radioName;
    MinosItem<PubSubName> antennaName;

    MinosStringItem<QString> VHFContestName;

    // Most of this info is bundle related, and not relevant when monitored (?)

    MinosStringItem<QString> entrant;
    MinosStringItem<QString> entSect;
    MinosStringItem<QString> sqth1;
    MinosStringItem<QString> sqth2;
    MinosStringItem<QString> entTx;
    MinosStringItem<QString> entRx;
    MinosStringItem<QString> entAnt;
    MinosStringItem<QString> entAGL;
    MinosStringItem<QString> entASL;
    MinosStringItem<QString> ops1;        // list of ops for contest
    MinosStringItem<QString> ops2;        // list of ops for contest
    MinosStringItem<QString> entCondx1;
    MinosStringItem<QString> entCondx2;
    MinosStringItem<QString> entCondx3;
    MinosStringItem<QString> entCondx4;
    MinosStringItem<QString> entName;
    MinosStringItem<QString> entCall;
    MinosStringItem<QString> entAddr1;
    MinosStringItem<QString> entAddr2;
    MinosStringItem<QString> entPostCode;
    MinosStringItem<QString> entCity;
    MinosStringItem<QString> entCountry;
    MinosStringItem<QString> entPhone;
    MinosStringItem<QString> entEMail;
    MinosStringItem<QString> sectionList; // needed to be carried forward

    MinosStringItem<QString> screenLayout;

};

#endif // CONTESTDETAILSTRANSFEROBJECT_H
