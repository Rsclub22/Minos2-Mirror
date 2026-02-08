/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2006 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include  <QKeyEvent>
#include "MinosLoggerEvents.h"

MinosLoggerEvents MinosLoggerEvents::mle;
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendContestPageChanged()
{
    emit mle.ContestPageChanged();
}

void MinosLoggerEvents::SendContestShownChanged()
{
    emit mle.ContestShownChanged();
}

void MinosLoggerEvents::SendContestBandChanged(BaseContestLog *ct)
{
    emit mle.ContestBandChanged(ct);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendColumnsChanged()
{
    emit mle.ColumnsChanged();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendDoColumnChanges(BaseContestLog *b)
{
    emit mle.doColumnChanges(b);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendSplittersChanged()
{
    emit mle.SplittersChanged();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendDoSplitterChanges(BaseContestLog *b)
{
    emit mle.doSplitterChanges(b);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendValidateError(int err)
{
    emit mle.ValidateError(err);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendReportOverstrike(bool ov, BaseContestLog *c)
{
    emit mle.ReportOverstrike(ov, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendAfterLogContact(BaseContestLog *ct, QSharedPointer<BaseContact> lct)
{
    emit mle.AfterLogContact(ct, lct);
}

//---------------------------------------------------------------------------
void MinosLoggerEvents::SendAfterSelectContact(QSharedPointer<BaseContact> lct, BaseContestLog *c)
{
    emit mle.AfterSelectContact(lct, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendGoToSerial(BaseContestLog *c)
{
    emit mle.GoToSerial(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendMakeEntry(BaseContestLog *c, bool e)
{
    emit mle.MakeEntry(c, e);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendNextUnfilled(BaseContestLog *c)
{
    emit mle.NextUnfilled(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendClearContestInFrame(BaseContestLog *c)
{
    emit mle.clearContestInFrame(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendTabSandP(BaseContestLog *c)
{
    emit mle.tabSandP(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendShowAuxHeaders()
{
    emit mle.showAuxHeaders();
}
void MinosLoggerEvents::SendShowCribBand()
{
    emit mle.ShowCribBand();
}
void MinosLoggerEvents::SendRedrawQSOMap(bool grid, bool lines
                                         , bool cluster
                                         , bool showloc, QString tl, QString br, bool showNav)
{
    emit mle.redrawQSOMap(grid, lines, cluster, showloc, tl, br, showNav);
}
void MinosLoggerEvents::sendBandmapLimitsChanged()
{
    emit mle.bandMapLimitsChanged();
}

void MinosLoggerEvents::SendFKey(BaseContestLog *c, int event, int mfreq)
{
    emit mle.fKey(c, event, mfreq);
}

void MinosLoggerEvents::SendSandPChanged(BaseContestLog *c, bool s)
{
    emit mle.SandPChanged(c, s);
}
void MinosLoggerEvents::SendDMMess(AnalysePubSubNotify an)
{
    emit mle.DMMess(an);
}

void MinosLoggerEvents::SendBroadcastSpot(QSharedPointer<ClusterSpotData> sd, bool delSpot)
{
    emit mle.broadcastSpots(sd, delSpot);
}

void MinosLoggerEvents::SendModeChange(QString mode)
{
    emit mle.modeChange(mode);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendMainRaised()
{
    emit mle.MainRaised();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendQSOMargins()
{
    emit mle.QSOMargins();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendScreenContactChanged(ScreenContact *sct, BaseContestLog *c, QString b)
{
    emit mle.ScreenContactChanged(sct, c, b);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendReplaceThisLogList(SharedMatchCollection matchCollection, BaseContestLog *c, QString b)
{
    emit mle.ReplaceThisLogList(matchCollection, c, b);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendReplaceOtherLogList(SharedMatchCollection matchCollection, BaseContestLog *c, QString b)
{
    emit mle.ReplaceOtherLogList(matchCollection, c, b);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendReplaceListList(SharedMatchCollection matchCollection, BaseContestLog *c, QString b)
{
    emit mle.ReplaceListList(matchCollection, c, b);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendScrollToCountry(QString prefix, BaseContestLog *c)
{
    emit mle.ScrollToCountry(prefix, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendScrollToDistrict(QString prefix, BaseContestLog *c)
{
    emit mle.ScrollToDistrict(prefix, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendMatchStarting( BaseContestLog *c)
{
    emit mle.MatchStarting(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendShowOperators()
{
    emit mle.ShowOperators();
}
//---------------------------------------------------------------------------
//void MinosLoggerEvents::SendBandMapPressed()
//{
//    emit mle.BandMapPressed();
//}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendTimerDistribution()
{
    emit mle.TimerDistribution();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendAfterTabFocusIn(QLineEdit *tle)
{
    emit mle.AfterTabFocusIn(tle);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendCountrySelect(QString cty, BaseContestLog *c)
{
    emit mle.CountrySelect(cty, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendDistrictSelect(QString dist, BaseContestLog *c)
{
    emit mle.DistrictSelect(dist, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendLocSelect(QString loc, BaseContestLog *c)
{
    emit mle.LocSelect(loc, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendRefreshStackMults(BaseContestLog *contest)
{
    emit mle.refreshStackMults(contest);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendSetMemory(BaseContestLog * ct, QString call, QString loc)
{
    emit mle.setMemory(ct, call, loc);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendSetMemoryAction(BaseContestLog * ct, QString call, QString loc)
{
    emit mle.setMemoryAction(ct, call, loc);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendFontChanged()
{
    emit mle.FontChanged();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendFiltersChanged(BaseContestLog *c)
{
    emit mle.FiltersChanged(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendUpdateStats(BaseContestLog *c )
{
    emit mle.UpdateStats(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendUpdateMemories(BaseContestLog *c )
{
    emit mle.UpdateMemories(c);
}

//---------------------------------------------------------------------------
void MinosLoggerEvents::SendRigFreqChanged(Frequency f, BaseContestLog *c)
{
    emit mle.RigFreqChanged(f, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendFreqToRig(Frequency f)
{
    emit mle.FreqToRig(f);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendBrgStrToRot(QString bearing)
{
    emit mle.BrgStrToRot(bearing);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendMatchBrgStrToRot(QString bearing)
{
    emit mle.MatchBrgStrToRot(bearing);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendSpotBrgStrToRot(QString bearing)
{
    emit mle.SpotBrgStrToRot(bearing);

}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendMemBrgStrToRot(QString bearing)
{
    emit mle.MemBrgStrToRot(bearing);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendSpotToLog(memoryData::memData spotData)
{
    emit mle.DxSpotToLog(spotData);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendQRZInfoToLog(QString callsign, QString locator, QString name)
{
    emit mle.QRZInfoToLog(callsign, locator, name);
}
void MinosLoggerEvents::SendRequestResendSpotsToClusterServer(resendFrameId frameId, QString command, QString bandmask, QString uuid)
{
    emit mle.ResendSpotsFromClusterCommand(frameId, command, bandmask, uuid);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendSpotToMemory(BaseContestLog *c, memoryData::memData spotData)
{
    emit mle.DxSpotToMemory(c, spotData);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendReconnectFlagToClusterServer(bool state)
{
    emit mle.SendReconnectFlagToServer(state);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendXferEnabled(bool s, BaseContestLog *c, QString basename)
{
    emit mle.XferEnabled(s, c, basename);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendXferPressed(BaseContestLog *c, QString basename)
{
    emit mle.XferPressed(c, basename);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendMatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, QItemSelection selected)
{
    emit mle.MatchTreeSelected(m, c, basename, selected);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendListCompressionChanged(qreal hmult)
{
    emit mle.listCompressionChanged(hmult);
}
//---------------------------------------------------------------------------

void MinosLoggerEvents::SendAfterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct)
{
    emit mle.afterQSOSaved(c, tct);
}

void MinosLoggerEvents::SendWsjtxDatagram(int instance, QByteArray *datagram)
{
    emit mle.wsjtxDatagram(instance, datagram);
}

void MinosLoggerEvents::SendCallsignLookup(BaseContestLog *l, QString c)
{
    emit mle.callsignLookup(l, c);
}
