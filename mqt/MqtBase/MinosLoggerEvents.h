/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2006 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef MinosLoggerEventsH
#define MinosLoggerEventsH

#include "base_pch.h"
#include "MatchCollection.h"
#include "rigmemcommondata.h"
//---------------------------------------------------------------------------
class BaseContestLog;
class LoggerContestLog;
class BaseContact;
class ScreenContact;
class TMatchCollection;
class QLineEdit;

class MinosLoggerEvents:public QObject
{
    Q_OBJECT
public:
    virtual ~MinosLoggerEvents() override
    {}

signals:
   void ContestPageChanged();
   void ColumnsChanged();
   void doColumnChanges(BaseContestLog *);
   void SplittersChanged();
   void doSplitterChanges(BaseContestLog *);
   void ValidateError(int err);
   void ReportOverstrike(bool ov, BaseContestLog *c);
   void AfterLogContact(BaseContestLog *ct);
   void AfterLogContactToCluster(BaseContestLog *ct, Callsign cs, QString loc);
   void AfterLogContactToBandmap(BaseContestLog *ct, Callsign cs, QString loc, QString bearing, QString freq);
   void AfterSelectContact(QSharedPointer<BaseContact> ct, BaseContestLog *);
   void ContestDetails(BaseContestLog *);
   void GoToSerial(BaseContestLog *);
   void MakeEntry(BaseContestLog *);
   void NextUnfilled(BaseContestLog *);
   void FormKey(unsigned int *, BaseContestLog *);
   void ScreenContactChanged(ScreenContact *, BaseContestLog *, QString b);
   void ReplaceThisLogList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
   void ReplaceOtherLogList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
   void ReplaceListList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
   void ScrollToCountry(QString prefix, BaseContestLog *);
   void ScrollToDistrict(QString prefix, BaseContestLog *);
   void MatchStarting(BaseContestLog *);
   void ShowOperators();
   //void BandMapPressed();
   void TimerDistribution();
   void AfterTabFocusIn(QLineEdit *tle);

   void BrgStrToRot(QString);
   void FreqStrToRig(QString);
   void MatchBrgStrToRot(QString);
   void SpotBrgStrToRot(QString);
   void MemBrgStrToRot(QString);
   void SendResendSpotsCommand(QString);
   void DxSpotToLog(memoryData::memData);         // cluster spot

   void DxSpotToMemory(BaseContestLog *, memoryData::memData);      // cluster spot


   void CountrySelect(QString cty, BaseContestLog *c);
   void DistrictSelect(QString dist, BaseContestLog *c);
   void LocSelect(QString loc, BaseContestLog *c);
   void refreshStackMults(BaseContestLog *contest);

   void setMemory(BaseContestLog *, QString call, QString loc);
   void setMemoryAction(BaseContestLog *, QString call, QString loc);


   void FiltersChanged(BaseContestLog *);
   void UpdateStats(BaseContestLog *);
   void UpdateMemories(BaseContestLog *);
   void RigFreqChanged(QString, BaseContestLog *);
   void RotBearingChanged(int, BaseContestLog *);

   void XferEnabled(bool s, BaseContestLog *c, QString basename);
   void XferPressed(BaseContestLog *c, QString basename);
   void MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, QItemSelection selected);

   void FontChanged();
   void listCompressionChanged(qreal hmult);

   void afterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct);
   void wsjtxDatagram(QByteArray *);
   void callsignLookup(BaseContestLog *l, QString c);
   void ResendSpotsFromClusterCommand(QString, int, QString);
public:
   static MinosLoggerEvents mle;

   static void SendContestPageChanged();
   static void SendColumnsChanged();
   static void SendDoColumnChanges(BaseContestLog *);
   static void SendSplittersChanged();
   static void SendDoSplitterChanges(BaseContestLog *);
   static void SendValidateError(int err);
   static void SendReportOverstrike(bool ov, BaseContestLog *c);
   static void SendAfterLogContact(BaseContestLog *ct);
   static void SendAfterLogContactToCluster(BaseContestLog *ct, Callsign cs, QString loc);
   static void SendAfterLogContactToBandmap(BaseContestLog *ct, Callsign cs, QString loc, QString bearing, QString freq);
   static void SendAfterSelectContact(QSharedPointer<BaseContact> ct, BaseContestLog *);
   static void SendContestDetails(BaseContestLog *);
   static void SendGoToSerial(BaseContestLog *);
   static void SendMakeEntry(BaseContestLog *);
   static void SendNextUnfilled(BaseContestLog *);
   static void SendFormKey(unsigned int *, BaseContestLog *);

   static void SendScreenContactChanged(ScreenContact *, BaseContestLog *, QString);
   static void SendReplaceThisLogList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
   static void SendReplaceOtherLogList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
   static void SendReplaceListList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
   static void SendScrollToCountry(QString prefix, BaseContestLog *);
   static void SendScrollToDistrict(QString prefix, BaseContestLog *);
   static void SendMatchStarting(BaseContestLog *);
   static void SendShowOperators();
   static void SendBandMapPressed();
   static void SendTimerDistribution();
   static void SendAfterTabFocusIn(QLineEdit *tle);

   static void SendCountrySelect(QString cty, BaseContestLog *c);
   static void SendDistrictSelect(QString dist, BaseContestLog *c);
   static void SendLocSelect(QString loc, BaseContestLog *c);
   static void sendRefreshStackMults(BaseContestLog *contest);
   static void sendSetMemory(BaseContestLog *, QString call, QString loc);
   static void sendSetMemoryAction(BaseContestLog *, QString call, QString loc);

   static void SendFontChanged();
   static void SendBrgStrToRot(QString);
   static void SendFreqStrToRig(QString);
   static void SendMatchBrgStrToRot(QString);
   static void SendSpotBrgStrToRot(QString);
   static void SendMemBrgStrToRot(QString);
   static void SendSpotToLog(memoryData::memData);     // cluster spot
   static void SendRequestResendSpotsToClusterServer(QString, int, QString);
   static void SendSpotToMemory(BaseContestLog *,memoryData::memData);  // cluster spot

   static void sendUpdateStats(BaseContestLog *c );
   static void sendFiltersChanged(BaseContestLog *c );
   static void sendUpdateMemories(BaseContestLog *c );

    static void sendRigFreqChanged(QString f, BaseContestLog *c);
    static void sendRotBearingChanged(int f, BaseContestLog *c);

    static void sendXferEnabled(bool s, BaseContestLog *c, QString basename);
    static void sendXferPressed(BaseContestLog *c, QString basename);
    static void sendMatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, QItemSelection selected);
    static void sendListCompressionChanged(qreal hmult);

    static void sendAfterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct);
    static void sendWsjtxDatagram(QByteArray *);
    static void sendCallsignLookup(BaseContestLog *, QString);

};
//---------------------------------------------------------------------------
#endif
