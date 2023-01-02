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

#include <QItemSelection>
#include <QKeyEvent>

#include "AnalysePubSubNotify.h"
#include "MatchCollection.h"
#include "baseconstants.h"
#include "rigmemcommondata.h"
#include "clustercommon.h"
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
   void ContestShownChanged();
   void ContestBandChanged(BaseContestLog *);
   void ColumnsChanged();
   void doColumnChanges(BaseContestLog *);
   void SplittersChanged();
   void doSplitterChanges(BaseContestLog *);
   void ValidateError(int err);
   void ReportOverstrike(bool ov, BaseContestLog *c);
   void AfterLogContact(BaseContestLog *ct);
   void AfterLogContactToCluster(BaseContestLog *ct, QSharedPointer<BaseContact> lct);
   void AfterLogContactToBandmap(BaseContestLog *ct, QSharedPointer<BaseContact> lct);
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
   void TimerDistribution();
   void AfterTabFocusIn(QLineEdit *tle);
   void clearContestInFrame(BaseContestLog *);
   void tabSandP();
   void MainRaised();
   void QSOMargins();
   void showAuxHeaders();
   void bandMapLimitsChanged();
   void fKey(BaseContestLog *c, int e);
   void redrawQSOMap(bool grid, bool lines, bool cluster, int cldist);

   void BrgStrToRot(QString);
   void FreqToRig(Frequency);
   void MatchBrgStrToRot(QString);
   void SpotBrgStrToRot(QString);
   void MemBrgStrToRot(QString);
   void SendResendSpotsCommand(QString);
   void SendReconnectFlagToServer(bool);
   void DxSpotToLog(memoryData::memData);         // cluster spot
   void DxSpotToMemory(BaseContestLog *, memoryData::memData);      // cluster spot
   void QRZInfoToLog(QString callsign, QString locator, QString name);

   void CountrySelect(QString cty, BaseContestLog *c);
   void DistrictSelect(QString dist, BaseContestLog *c);
   void LocSelect(QString loc, BaseContestLog *c);
   void refreshStackMults(BaseContestLog *contest);

   void setMemory(BaseContestLog *, QString call, QString loc);
   void setMemoryAction(BaseContestLog *, QString call, QString loc);


   void FiltersChanged(BaseContestLog *);
   void UpdateStats(BaseContestLog *);
   void UpdateMemories(BaseContestLog *);
   void RigFreqChanged(Frequency, BaseContestLog *);
   void RotBearingChanged(int, BaseContestLog *);

   void XferEnabled(bool s, BaseContestLog *c, QString basename);
   void XferPressed(BaseContestLog *c, QString basename);
   void MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, QItemSelection selected);

   void FontChanged();
   void listCompressionChanged(qreal hmult);

   void afterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct);
   void wsjtxDatagram(int instance, QByteArray *);
   void callsignLookup(BaseContestLog *l, QString c);
   void ResendSpotsFromClusterCommand(resendFrameId, QString, QString, QString);
   void SandPChanged(bool);
   void DMMess(AnalysePubSubNotify);
public:
   static MinosLoggerEvents mle;

   static void SendContestPageChanged();
   static void SendContestShownChanged();
   static void SendContestBandChanged(BaseContestLog *);
   static void SendColumnsChanged();
   static void SendDoColumnChanges(BaseContestLog *);
   static void SendSplittersChanged();
   static void SendDoSplitterChanges(BaseContestLog *);
   static void SendValidateError(int err);
   static void SendReportOverstrike(bool ov, BaseContestLog *c);
   static void SendAfterLogContact(BaseContestLog *ct);
   static void SendAfterLogContactToCluster(BaseContestLog *ct, QSharedPointer<BaseContact> lct);
   static void SendAfterLogContactToBandmap(BaseContestLog *ct, QSharedPointer<BaseContact> lct);
   static void SendAfterSelectContact(QSharedPointer<BaseContact> ct, BaseContestLog *);
   static void SendContestDetails(BaseContestLog *);
   static void SendGoToSerial(BaseContestLog *);
   static void SendMakeEntry(BaseContestLog *);
   static void SendNextUnfilled(BaseContestLog *);
   static void SendFormKey(unsigned int *, BaseContestLog *);
   static void SendClearContestInFrame(BaseContestLog *);
   static void SendTabSandP();
   static void SendMainRaised();
   static void SendQSOMargins();

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
   static void SendRefreshStackMults(BaseContestLog *contest);
   static void SendSetMemory(BaseContestLog *, QString call, QString loc);
   static void SendSetMemoryAction(BaseContestLog *, QString call, QString loc);

   static void SendFontChanged();
   static void SendBrgStrToRot(QString);
   static void SendFreqToRig(Frequency);
   static void SendMatchBrgStrToRot(QString);
   static void SendSpotBrgStrToRot(QString);
   static void SendMemBrgStrToRot(QString);
   static void SendSpotToLog(memoryData::memData);     // cluster spot
   static void SendRequestResendSpotsToClusterServer(resendFrameId, QString, QString, QString);
   static void SendSpotToMemory(BaseContestLog *,memoryData::memData);  // cluster spot

   static void SendUpdateStats(BaseContestLog *c );
   static void SendFiltersChanged(BaseContestLog *c );
   static void SendUpdateMemories(BaseContestLog *c );

    static void SendRigFreqChanged(Frequency f, BaseContestLog *c);
    static void SendRotBearingChanged(int f, BaseContestLog *c);

    static void SendXferEnabled(bool s, BaseContestLog *c, QString basename);
    static void SendXferPressed(BaseContestLog *c, QString basename);
    static void SendMatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, QItemSelection selected);
    static void SendListCompressionChanged(qreal hmult);

    static void SendAfterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct);
    static void SendWsjtxDatagram(int instance, QByteArray *);
    static void SendCallsignLookup(BaseContestLog *, QString);

    static void SendReconnectFlagToClusterServer(bool state);
    static void SendQRZInfoToLog(QString callsign, QString locator, QString name);
    static void SendShowAuxHeaders();
    static void sendBandmapLimitsChanged();

    static void SendRedrawQSOMap(bool grid, bool lines, bool cluster, int cldist);

    static void SendFKey(BaseContestLog *c, int event);
    static void SendSandPChanged(bool);
    static void SendDMMess(AnalysePubSubNotify);
};
//---------------------------------------------------------------------------
#endif
