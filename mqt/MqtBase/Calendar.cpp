#include "base_pch.h"
#include "cutils.h"
#include "BandList.h"
#include "Calendar.h"

const QString monthTable[ 12 ] =
    {
        "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
static int monthLength[ 12 ] =
    {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
//const QString days[ 7 ] =
//    {
//        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
//    };

const char * Calendar::TypeVHFContest = QT_TR_NOOP("<VHF from VHFContests.xml>");
const char * Calendar::TypeMwaveContest = QT_TR_NOOP("<Microwave from MicroContestsxx.xml>");

QString Calendar::getTypeName ( const QString &xmlName, CalType calType )
{

    QMap<QString, QString>::iterator ctype = contestNameMap.find( xmlName );
    if ( ctype != contestNameMap.end() )
    {
        return ctype.value();
    }

    if ( calType == ectVHF || calType == ectVHFOther )
    {
        return tr(TypeVHFContest);
    }
    if ( calType == ectMwave )
    {
        return tr(TypeMwaveContest);
    }

    return tr("Undefined");
}

void Calendar::setYear ( int y )
{
    curYear = y;
    if ( y % 4 == 0 )
    {
        monthLength[ 1 ] = 29;
    }
    else
    {
        monthLength[ 1 ] = 28;
    }
}
int Calendar::getMonth ( const QString &m )
{
    QString m2 = m.toUpper().left ( 3 );
    for ( int i = 0; i < 12; i++ )
    {
        if ( m2 == monthTable[ i ] )
        {
            return i + 1;
        }
    }
    return 0;
}
int Calendar::getDayOfWeek ( QDateTime dtg )
{
    int dw = dtg.date().dayOfWeek();    // Monday is 1, Sunday is 7
    return dw;
}
int Calendar::getDate ( int month, int day, int week )
{
    // Day 1 is Monday

    // day 6 week 1 means "first Saturday of the month"
    // and day 2 week 1 means "first Tuesday of the month".
    // week 2 day 2 would mean "2nd Tuesday of the month".
    // day 6+1 is Sunday after Saturday

    // day 1+3 week 3 is  Thursday after 3rd Monday

    // Sept 2010 day 4+13 week 2 is 13 days after 2nd Thursday (Sep 22 2010)

    // return the date of the month/day/week


#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QDateTime startMonth ( QDate( curYear, month, 1 ) );
#else
    QDateTime startMonth  = QDate( curYear, month, 1 ).startOfDay(Qt::UTC);
#endif
    int dw = getDayOfWeek ( startMonth ) - 1;   // make it 0 based

    // So now we need to go through
    int wcount = 1;   // start in first week
    for ( int i = 0; i < monthLength[ month - 1 ]; i++ )
    {
        if ( ( dw + i ) % 7 == day - 1 )
        {
            // we are on the correct day
            if ( wcount == week )
            {
                return i + 1;
            }
            wcount++;
        }
    }

    return 0;
}

Calendar::Calendar ( int year, CalType calType ) : calType ( calType )
{
    calendarYear = year;
}

Calendar::~Calendar()
{}

/*
The 9th EC Directive prescribes the start and end dates of summer time as the
last Sundays in March and October respectively. These dates are in line with
those already operating in the United Kingdom. The 9th Directive provides
that these start and end dates should apply indefinitely.
*/


QDateTime Calendar::localToUTC( QDateTime t )
{
    int month = t.date().month();
    int day = t.date().day();

    bool isDst = false;

    if ( month > 3 && month < 10 )
    {
        isDst = true;
    }
    else
        if ( month == 3 )
        {
            int fSundayApril = getDate ( 4, 7, 1 );
            int ldM = fSundayApril + 31 - 7 ;
            if ( day >= ldM )
            {
                isDst = true;
            }
        }
        else
            if ( month == 10 )
            {
                int fSundayNovember = getDate ( 11, 7, 1 );
                int lsO = fSundayNovember + 31 - 7 ;
                if ( day < lsO )
                {
                    isDst = true;
                }
            }
    if ( isDst )
    {
        t = t.addSecs( -3600 );       // subtract 1 hour
    }
    return t;
}

bool Calendar::parseFile ( const QString &fname )
{
    QFile file( fname );

    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return false;
    }

    QString buffer2;
    {
        // scoping only
        QByteArray total = file.readAll();

        QString buffer = QString( total );


        int dtdPos = buffer.indexOf ( "<!DOCTYPE" );
        if ( dtdPos != -1 )
        {
            buffer2 = buffer.left (dtdPos );

            int dtdEndPos = buffer.indexOf ( "]>" );
            if ( dtdEndPos == -1 )
            {
                return false;
            }
            buffer2 += buffer.mid ( dtdEndPos + 2, buffer.size() - dtdEndPos - 2 );
        }
        else
        {
            buffer2 = buffer;
        }
    }
    TiXmlBase::SetCondenseWhiteSpace ( false );
    TiXmlDocument xdoc;
    const char *loaded = xdoc.Parse ( buffer2.toStdString().c_str() );
    if (!loaded)
    {
        return false;
    }
    TiXmlElement *tix = xdoc.RootElement();
    if ( !tix || !checkElementName ( tix, "calendar" ) )
    {
        return false;
    }
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "version" ) )
        {
            if ( !parseVersion ( e ) )
            {
                return false;
            }
        }
        if ( checkElementName ( e, "mult_type" ) )
        {
            if ( !parseMultType ( e ) )
            {
                return false;
            }
        }
        else
            if ( checkElementName ( e, "special_rule" ) )
            {
                if ( !parseSpecialRule ( e ) )
                {
                    return false;
                }
            }
            else
                if ( checkElementName ( e, "section" ) )
                {
                    if ( !parseSection ( e ) )
                    {
                        return false;
                    }
                }
                else
                    if ( checkElementName ( e, "band" ) )
                    {
                        if ( !parseBand ( e ) )
                        {
                            return false;
                        }
                    }
                    else
                        if ( checkElementName ( e, "contest_series" ) )
                        {
                            if ( !parseContestSeries ( e ) )
                            {
                                return false;
                            }
                        }
                        else
                            if ( checkElementName ( e, "contest" ) )
                            {
                                if ( !parseContest ( e ) )
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                QString eval = e->Value();
                                //<!ELEMENT calendar (version, icalurl?, googleurl?, notes_list*, mult_type+, special_rule+, category*, section+, band+, contest_series*, contest+)>
                                continue;
                            }
    }
    // Now we have the raw info; we need to go through it and
    // generate all the individual contest details for logger/adjsql
    setYear ( calendarYear );
    for ( QMap<QString, CalendarContest>::iterator i = contests.begin(); i != contests.end(); i++ )
    {

        for ( QVector<CalendarBandList>::iterator bl = i.value().bandList.begin(); bl != i.value().bandList.end(); bl++ )
        {
            // For each contest iterate the time list or the band list
            // the band list overrides the time list!

            QVector<TimeList>::iterator tls;
            QVector<TimeList>::iterator tle;

            CalendarBand &cb = bands[ (*bl).name ];

            if (cb.timeList.size() != 0)
            {
                tls = cb.timeList.begin();
                tle = cb.timeList.end();
            }
            else
            {
                tls = i.value().timeList.begin();
                tle = i.value().timeList.end();
            }

            for ( QVector<TimeList>::iterator tl = tls; tl != tle; tl++ )
            {
                for ( QVector<MonthList>::iterator ml = ( *tl ).monthList.begin(); ml != ( *tl ).monthList.end(); ml++ )
                {
                    // startDate needs to incorporate the week and day

                    // day 6 week 1 means "first Saturday of the month"
                    // and day 2 week 1 means "first Tuesday of the month".
                    // week 2 day 2 would mean "2nd Tuesday of the month".
                    // day 6+1 is Sunday after Saturday

                    // XMAS cumulatives also have <start_date_list>, but they appear to be the only ones


                    int sm = getMonth ( ( *ml ).month );
                    if ( sm == 0 )
                    {
                        continue;
                    }

                    // we need to copy the datelist before we add to it...

                    QVector<StartDateList> startDateList = ( *tl ).startDateList;

                    if ( startDateList.size() == 0 )
                    {
                        // make the start info into a statt date (just one!)

                        QString startWeek = ( *tl ).startWeek;
                        QString startDay = ( *tl ).startDay;


                        QString startday1 = startDay;
                        QString startday2;
                        int istartday1, istartday2;

                        int ppos = startDay.indexOf ( "+" );
                        if ( ppos != -1 )
                        {
                            startday1 = startDay.left ( ppos );
                            startday2 = startDay.mid ( ppos + 1, startDay.size() );
                            istartday1 = startday1.toInt();
                            istartday2 = startday2.toInt();
                        }
                        else
                        {
                            istartday1 = startday1.toInt();
                            istartday2 = 0;
                        }
                        if ( istartday1 == 0 )
                        {
                            continue;   // next time list
                        }

                        int istartWeek = startWeek.toInt();
                        if ( istartWeek == 0 )
                        {
                            continue;   // next time list
                        }


                        //               QString bstartWeek = (*bl).startWeek; // or iterate its timeList
                        int istartDate = getDate ( sm, istartday1, istartWeek );
                        if ( istartDate == 0 )
                        {
                            continue;   // next time list
                        }

                        istartDate += istartday2;

                        StartDateList sdl;
                        sdl.date = QString::number(istartDate);

                        startDateList.push_back(sdl);
                    }

                    if ( startDateList.size() )
                    {
                        // need to iterate the start dates
                        for ( int j = 0; j < startDateList.size(); j++ )
                        {
                            int istartDate = startDateList[ j ].date.toInt();
                            if ( istartDate == 0 )
                            {
                                continue;
                            }

                            IndividualContest ic;

                            QString desc = i.value().description.trimmed();
                            QString sdesc = i.value().shortDescription.trimmed();
                            QString name = i.value().name.trimmed();
                            QString typeName = getTypeName ( name, calType );
                            QString mode = i.value().mode.trimmed();

                            // This needs changing once the contests are sorted
                            // as e.g. 70MHz cumulatives are defined in two groups
                            //if (desc[desc.size() - 1] == '#')
                            //{
                            //   desc += QString::number(instance++);
                            //}
                            ic.name = name;
                            ic.typeName = typeName;
                            ic.description = desc;
                            ic.shortDescription = sdesc;
                            ic.bands = ( *bl ).name;
                            ic.mode = mode;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
                            ic.start = QDateTime ( QDate( curYear, sm, istartDate ) );
#else
                            ic.start = QDate( curYear, sm, istartDate ).startOfDay(Qt::UTC);
#endif
                            int h = ( *tl ).startTime.left( 2 ).toInt();
                            int m = ( *tl ).startTime.mid( 2, 2 ).toInt();

                            ic.start = ic.start.addSecs( h * 3600 + m * 60 );

                            ic.duration = ( *tl ).duration;
                            double dur = ic.duration.toDouble();
                            double durh = static_cast< int >( dur / 24 );
                            double durm = ( dur - durh ) * 60;
                            ic.finish = ic.start.addSecs( static_cast< int >(durh) * 3600 + static_cast< int >(durm) * 60 );

                            QString timeType = ( *tl ).timeType;
                            if ( timeType == "local" )
                            {
                                ic.start = localToUTC ( ic.start );
                                ic.finish = localToUTC ( ic.finish );
                            }

                            ic.ppKmScoring = ( i.value().scoring == CalendarContest::perkms );
                            for ( int j = 0; j < i.value().sectionList.size(); j++ )
                            {
                                QString n = i.value().sectionList[ j ].name;

                                QMap<QString, CalendarSection>::iterator s = sections.find ( n );

                                if ( s == sections.end() )
                                {
                                    continue;
                                }
                                int mls = s.value().monthList.size();
                                if ( mls )
                                {
                                    bool monthOK = false;
                                    for ( QVector<MonthList>::iterator ml = s.value().monthList.begin(); ml != s.value().monthList.end(); ml++ )
                                    {
                                        if ( ( *ml ).month.compare(monthTable[ sm - 1 ], Qt::CaseInsensitive) == 0 )
                                        {
                                            monthOK = true;
                                            break;
                                        }
                                    }
                                    if ( !monthOK )
                                    {
                                        continue;
                                    }
                                }

                                if ( !sections[ n ].overall )
                                {
                                    ic.sections += n;
                                    ic.sections += ",";
                                }
                            }
                            ic.sections = ic.sections.left ( ic.sections.size() - 1 );  // lose any trailing comma
                            ic.mults = i.value().mult;
                            for ( int j = 0; j < i.value().specialRulesList.size(); j++ )
                            {
                                ic.specialRules += i.value().specialRulesList[ j ].name + " ";
                            }
                            ic.power = i.value().power;

                            ic.reg1band = bands[ ic.bands ].reg1band;
                            if ( ic.reg1band == "1,2 GHz" )
                            {
                                ic.reg1band = "1,3 GHz";
                            }

                            BandList &blist = BandList::getBandList();
                            QSharedPointer<BandInfo>  bi;
                            blist.findBand(ic.reg1band, bi);
                            ic.flow = bi->fLow;
                            ic.ukband = bi->uk;

                            calendar.push_back ( ic );
                        }
                    }
                }
            }
        }
        // Now correct the power for the overall contest
        // (the band NAME is against each level, we want the canonical band)
        // and for each section
    }
    std::sort(calendar.begin(), calendar.end() );
    return true;

}
bool IndividualContest::operator< ( const IndividualContest& rhs ) const
{
    // p1 is from list; p2 is the one being searched for

    if ( start != rhs.start )
    {
        return start < rhs.start;
    }
    if (!almost_equal( flow, rhs.flow, 2))
    {
        return flow <  rhs.flow ;
    }
    if ( reg1band != rhs.reg1band )
    {
        return reg1band < rhs.reg1band;
    }
    return description < rhs.description;
}
bool Calendar::parseVersion ( TiXmlElement * tix )
{
    version = tix->GetText();
    return true;
}
bool Calendar::parseMultType ( TiXmlElement * tix )
{
    MultType mt;
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "name" ) )
        {
            mt.name = e->GetText();
        }
        else
            if ( checkElementName ( e, "description" ) )
            {
                mt.description = e->GetText();
            }
            else
                if ( checkElementName ( e, "long_description" ) )
                {
                    mt.longDescription = e->GetText();
                }
                else
                    if ( checkElementName ( e, "exchange" ) )
                    {
                        mt.exchange = e->GetText();
                    }
                    else
                        if ( checkElementName ( e, "scoring_description" ) )
                        {
                            mt.scoringDescription = e->GetText();
                        }
                        else
                            if ( checkElementName ( e, "bonus" ) )
                            {
                                //<bonus square="IO83" points="500"/>
                                QString square = e->Attribute("square");
                                QString points = e->Attribute("points");

                                mt.bonuses[square] = points.toInt();
                            }
                            else
                                {
                                    QString eval = e->Value();
                                    continue;
                                }
    }
    mults[ mt.name ] = mt;
    return true;
}
bool Calendar::parseSpecialRule ( TiXmlElement * tix )
{
    SpecialRule sr;
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "name" ) )
        {
            sr.name = e->GetText();
        }
        else
            if ( checkElementName ( e, "description" ) )
            {
                sr.description = e->GetText();
            }
            else
                if ( checkElementName ( e, "long_description" ) )
                {
                    sr.longDescription = e->GetText();
                }
                else
                {
                    QString eval = e->Value();
                    continue;
                }
    }
    specialRules[ sr.name ] = sr;
    return true;
}
bool Calendar::parseSection ( TiXmlElement * tix )
{
    CalendarSection s;
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "name" ) )
        {
            s.name = e->GetText();
        }
        else
            if ( checkElementName ( e, "description" ) )
            {
                s.description = e->GetText();
            }
            else
                if ( checkElementName ( e, "long_description" ) )
                {
                    s.longDescription = e->GetText();
                }
                else
                    if ( checkElementName ( e, "power" ) )
                    {
                        // POWER TO DO
                        // may be several entries - per band
                        QString band = getAttribute(e, "band");
                        s.maxPower[band] = e->GetText();
                    }
                    else
                        if (checkElementName ( e, "minpower" ))
                        {
                            // POWER TO DO
                            // may be several entries - per band
                            QString band = getAttribute(e, "band");
                            s.minPower[band] = e->GetText();
                        }
                        else
                            if ( checkElementName ( e, "height" ) )
                            {
                                s.height = e->GetText();
                            }
                            else
                                if ( checkElementName ( e, "singleantenna" ) )
                                {
                                    s.singleAntenna = ( QString(e->GetText()) == "1" );
                                }
                                else
                                    if ( checkElementName ( e, "overall_section" ) )
                                    {
                                        s.overall = true;
                                    }
                                    else
                                        if ( checkElementName ( e, "month_list" ) )
                                        {
                                            MonthList m;
                                            m.month = e->GetText();
                                            s.monthList.push_back ( m );
                                        }
                                        else
                                        {
                                            QString eval = e->Value();
                                            continue;
                                    }
    }
    sections[ s.name ] = s;
    return true;
}
bool Calendar::parseBand ( TiXmlElement * tix )
{
    CalendarBand b;
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "name" ) )
        {
            b.name = e->GetText();
        }
        else
            if ( checkElementName ( e, "reg1band" ) )
            {
                b.reg1band = e->GetText();
                if ( b.reg1band == "1,2 GHz" )
                {
                    b.reg1band = "1,3 GHz";
                }
            }
            else
                if ( checkElementName ( e, "wavelength" ) )
                {
                    b.wavelength = e->GetText();
                }
                else
                    if ( checkElementName ( e, "time_list" ) )
                    {
                        if ( ! ( parseTimeList ( e, b.timeList ) ) )
                            return false;
                    }
                    else
                    {
                        QString eval = e->Value();
                        continue;
                    }
    }
    bands[ b.name ] = b;
    return true;
}
bool Calendar::parseContestSeries ( TiXmlElement * tix )
{
    ContestSeries cs;
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "name" ) )
        {
            cs.name = e->GetText();
        }
        else
            if ( checkElementName ( e, "description" ) )
            {
                cs.description = e->GetText();
            }
            else
                if ( checkElementName ( e, "contest_list" ) )
                {
                    CalendarContestList cl;
                    cl.name = e->GetText();
                    cs.contestList.push_back ( cl );
                }
                else
                    if ( checkElementName ( e, "award_list" ) )
                    {
                        QString al;
                        al = e->GetText();
                        cs.awardList.push_back ( al );
                    }
                    else
                    {
                        QString eval = e->Value();
                        continue;
                    }
    }
    contestSeries[ cs.name ] = cs;
    return true;
}
bool Calendar::parseContest ( TiXmlElement * tix )
{
    CalendarContest c;
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "name" ) )
        {
            c.name = e->GetText();
        }
        else
            if ( checkElementName ( e, "description" ) )
            {
                c.description = e->GetText();
            }
            else
                if ( checkElementName ( e, "short_description" ) )
                {
                    c.shortDescription = e->GetText();
                }
                else
                    if ( checkElementName ( e, "time_list" ) )
                    {
                        if ( ! ( parseTimeList ( e, c.timeList ) ) )
                            return false;
                    }
                    else
                        if ( checkElementName ( e, "section_list" ) )
                        {
                            CalendarSectionList sl;
                            sl.name = e->GetText();
                            c.sectionList.push_back ( sl );
                        }
                        else
                            if ( checkElementName ( e, "category_list" ) )
                            {
                            }
                            else
                                if ( checkElementName ( e, "cabrillo_name" ) )
                                {
                                }
                                else
                                    if ( checkElementName ( e, "rst_type" ) )
                                    {
                                    }
                                    else
                                        if ( checkElementName ( e, "entry_email" ) )
                                        {
                                        }
                                        else
                                            if ( checkElementName ( e, "sprint" ) )
                                            {
                                            }
                                            else
                                                if ( checkElementName ( e, "special_rules_list" ) )
                                                {
                                                    SpecialRuleList srl;
                                                    srl.name = e->GetText();
                                                    c.specialRulesList.push_back ( srl );
                                                }
                                                else
                                                    if ( checkElementName ( e, "band_list" ) )
                                                    {
                                                        CalendarBandList bl;
                                                        bl.name = e->GetText();
                                                        c.bandList.push_back ( bl );
                                                    }
                                                    else
                                                        if ( checkElementName ( e, "award_list" ) )
                                                        {
                                                            QString al;
                                                            al = e->GetText();
                                                            c.awardList.push_back ( al );
                                                        }
                                                        else
                                                            if ( checkElementName ( e, "iaru" ) )
                                                            {
                                                                c.iaru = ( QString(e->GetText()) == "1" );
                                                            }
                                                            else
                                                                if ( checkElementName ( e, "scoring" ) )
                                                                {
                                                                    for ( TiXmlElement * f = e->FirstChildElement(); f; f = f->NextSiblingElement() )
                                                                    {
                                                                        if ( checkElementName ( f, "perkms" ) )
                                                                        {
                                                                            c.scoring = CalendarContest::perkms;
                                                                        }
                                                                        else
                                                                            if ( checkElementName ( f, "oneppq" ) )
                                                                            {
                                                                                c.scoring = CalendarContest::oneppq;
                                                                            }
                                                                            else
                                                                                if ( checkElementName ( f, "desc" ) )
                                                                                {
                                                                                    c.scoring = CalendarContest::desc;
                                                                                }
                                                                                else
                                                                                    if ( checkElementName ( e, "power" ) )
                                                                                    {
                                                                                        // POWER TO DO
                                                                                        // may be several entries - per band
                                                                                        QString band = getAttribute(e, "band");
                                                                                        // This gives the XML name for the band
                                                                                        c.maxPower[band] = e->GetText();
                                                                                    }
                                                                                    else
                                                                                        if (checkElementName ( e, "minpower" ))
                                                                                        {
                                                                                            // POWER TO DO
                                                                                            // may be several entries - per band
                                                                                            QString band = getAttribute(e, "band");
                                                                                            c.minPower[band] = e->GetText();
                                                                                        }
                                                                                        else
                                                                                            return false;
                                                                        break;
                                                                    }
                                                                }
                                                                else
                                                                    if ( checkElementName ( e, "mult" ) )
                                                                    {
                                                                        c.mult = e->GetText();
                                                                    }
                                                                    else
                                                                        if ( checkElementName ( e, "power" ) )
                                                                        {
                                                                            c.power = e->GetText();
                                                                        }
                                                                        else
                                                                            if ( checkElementName ( e, "mode" ) )
                                                                            {
                                                                                QString mode;
                                                                                mode = e->GetText();
                                                                                if (mode == "SSB")
                                                                                {
                                                                                   mode = hamlibData::USB;
                                                                                }
                                                                                if (mode == "CW")
                                                                                {
                                                                                   mode = hamlibData::CW;
                                                                                }
                                                                                if (mode == "FM")
                                                                                {
                                                                                   mode = hamlibData::FM;
                                                                                }
                                                                                c.mode = mode;
                                                                            }
                                                                            else
                                                                                if ( checkElementName ( e, "entry_date" ) )
                                                                                {
                                                                                    c.entryDate = e->GetText();
                                                                                }
                                                                                else
                                                                                {
                                                                                    QString eval = e->Value();
                                                                                    continue;
                                                                                }
    }
    contests[ c.name ] = c;
    return true;
}
bool Calendar::parseTimeList ( TiXmlElement * tix, QVector<TimeList> &timeList )
{
    TimeList t;
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        if ( checkElementName ( e, "month_list" ) )
        {
            MonthList m;
            m.month = e->GetText();
            t.monthList.push_back ( m );
        }
        else
            if ( checkElementName ( e, "start_date_list" ) )
            {
                StartDateList sdl;
                sdl.date = e->GetText();
                t.startDateList.push_back ( sdl );
            }
            else
                if ( checkElementName ( e, "start_day" ) )
                {
                    t.startDay = e->GetText();
                }
                else
                    if ( checkElementName ( e, "start_week" ) )
                    {
                        t.startWeek = e->GetText();
                    }
                    else
                        if ( checkElementName ( e, "start_time" ) )
                        {
                            t.startTime = e->GetText();
                        }
                        else
                            if ( checkElementName ( e, "time_type" ) )
                            {
                                t.timeType = e->GetText();
                            }
                            else
                                if ( checkElementName ( e, "duration" ) )
                                {
                                    t.duration = e->GetText();
                                }
                                else
                                    if ( checkElementName ( e, "mult" ) )
                                    {
                                        t.mult = e->GetText();
                                    }
                                    else
                                    {
                                        QString eval = e->Value();
                                        continue;
                                    }
    }
    timeList.push_back ( t );
    return true;
}
