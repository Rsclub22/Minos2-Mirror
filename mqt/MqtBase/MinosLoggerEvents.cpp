/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2006 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include "base_pch.h"
#include "MinosLoggerEvents.h"

MinosLoggerEvents MinosLoggerEvents::mle;
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendContestPageChanged()
{
    emit mle.ContestPageChanged();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendLogColumnsChanged()
{
    emit mle.LogColumnsChanged();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendSplittersChanged()
{
    emit mle.SplittersChanged();
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
void MinosLoggerEvents::SendAfterLogContact(BaseContestLog *ct)
{
    emit mle.AfterLogContact(ct);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendAfterSelectContact(QSharedPointer<BaseContact> lct, BaseContestLog *c)
{
    emit mle.AfterSelectContact(lct, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendContestDetails(BaseContestLog *c)
{
    emit mle.ContestDetails(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendGoToSerial(BaseContestLog *c)
{
    emit mle.GoToSerial(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendMakeEntry(BaseContestLog *c)
{
    emit mle.MakeEntry(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendNextUnfilled(BaseContestLog *c)
{
    emit mle.NextUnfilled(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendFormKey(unsigned int *key, BaseContestLog *c)
{
    emit mle.FormKey(key, c);
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
void MinosLoggerEvents::SendBandMapPressed()
{
    emit mle.BandMapPressed();
}
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
void MinosLoggerEvents::SendValidated()
{
    emit mle.Validated();
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
void MinosLoggerEvents::sendRefreshStackMults(LoggerContestLog *contest)
{
    emit mle.refreshStackMults(contest);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendSetMemory(BaseContestLog * ct, QString call, QString loc)
{
    emit mle.setMemory(ct, call, loc);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendFontChanged()
{
    emit mle.FontChanged();
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendFiltersChanged(BaseContestLog *c)
{
    emit mle.FiltersChanged(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendUpdateStats(BaseContestLog *c )
{
    emit mle.UpdateStats(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendUpdateMemories(BaseContestLog *c )
{
    emit mle.UpdateMemories(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendRefreshMults(BaseContestLog *c )
{
    emit mle.RefreshMults(c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendRigFreqChanged(QString f, BaseContestLog *c)
{
    emit mle.RigFreqChanged(f, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendRotBearingChanged(int f, BaseContestLog *c)
{
    emit mle.RotBearingChanged(f, c);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::SendBrgStrToRot(QString bearing)
{
    emit mle.BrgStrToRot(bearing);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendXferEnabled(bool s, BaseContestLog *c, QString basename)
{
    emit mle.XferEnabled(s, c, basename);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendXferPressed(BaseContestLog *c, QString basename)
{
    emit mle.XferPressed(c, basename);
}
//---------------------------------------------------------------------------
void MinosLoggerEvents::sendMatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, QItemSelection selected)
{
    emit mle.MatchTreeSelected(m, c, basename, selected);
}
//---------------------------------------------------------------------------
