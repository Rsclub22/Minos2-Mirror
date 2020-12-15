#ifndef CALENDAR_H
#define CALENDAR_H

#include "base_pch.h"

enum CalType {ectVHF, ectHF, ectMwave, ectVHFOther, ectHFOther, ectHFBARTG};
extern QMap<QString, QString> contestNameMap;

class MultType
{
    public:
        QString name;
        QString description;
        QString longDescription;
        QString scoringDescription;
        QString exchange;

        QMap<QString, int> bonuses;
};
class SpecialRule
{
    public:
        QString name;
        QString description;
        QString longDescription;
};
class MonthList
{
    public:
        QString month;
};
class CalendarSection
{
    public:
        CalendarSection() : singleAntenna ( false ), overall ( false )
        {}
        QVector<MonthList> monthList;

        QString name;
        QString description;
        QString longDescription;
        QMap<QString, QString> maxPower;
        QMap<QString, QString> minPower;
        QString height;
        bool singleAntenna;
        bool overall;
};
class StartDateList
{
    public:
        QString date;

};
class TimeList
{
    public:
        QVector<MonthList> monthList;
        QVector<StartDateList> startDateList;
        QString startDay;
        QString startWeek;
        QString startTime;
        QString timeType;
        QString duration;
        QString mult;
};
class CalendarBand
{
    public:
        QString name;
        QString reg1band;
        QString wavelength;
        QVector<TimeList> timeList;
};
class CalendarContestList
{
    public:
        QString name;

};
class ContestSeries
{
    public:
        QString name;
        QString description;
        QVector<CalendarContestList>contestList;
        QVector<QString>awardList;
};
class CalendarSectionList
{
    public:
        QString name;
        bool operator< ( const CalendarSectionList& rhs ) const
        {
            return name < rhs.name;
        }
        bool operator== ( const CalendarSectionList& rhs ) const
        {
            return name == rhs.name;
        }
};
class CalendarBandList
{
    public:
        QString name;

        QVector<CalendarSectionList> sectionList;
};
class SpecialRuleList
{
    public:
        QString name;

};
class CalendarContest
{
    public:
        CalendarContest() : scoring ( perkms ), iaru ( false )
        {}

        QString name;
        QString description;
        QString shortDescription;
        enum ScoreType{oneppq, perkms, desc} scoring;
        bool iaru;
        QString mult;
        QString power;
        QString mode;
        QString entryDate;

        QVector<TimeList> timeList;
        QVector<CalendarBandList> bandList;
        QVector<CalendarSectionList> sectionList;
        QVector<SpecialRuleList> specialRulesList;
        QVector<QString> awardList;

        QMap<QString, QString> maxPower;
        QMap<QString, QString> minPower;

};
class IndividualContest
{
    public:
        QDateTime start;
        QDateTime finish;
        QString duration;

        QString typeName;
        QString name;
        QString description;
        QString shortDescription;
        QString bands;
        QString reg1band;
        QString ukband;
        double flow = 0;
        bool ppKmScoring;
        QString sections;
        QString mults;
        QString specialRules;
        QString power;
        QString antenna;
        QString mode;

        bool operator< ( const IndividualContest& rhs ) const;
};
class TiXmlElement;
class Calendar
{
    Q_DECLARE_TR_FUNCTIONS(Calendar)

    static const char * TypeVHFContest;
    static const char * TypeMwaveContest;

        int curYear;
        int calendarYear;
        QString getTypeName ( const QString &xmlName, CalType calType );
        int getDayOfWeek ( QDateTime dtg );
        QDateTime localToUTC ( QDateTime t );
        void setYear ( int y );
        int getMonth ( const QString &m );
        int getDate ( int month, int day, int week );
    public:
        Calendar ( int year, CalType calType );
        ~Calendar();
        CalType calType;
        bool parseFile (const QString &fname );
        bool parseVersion ( TiXmlElement * e );
        bool parseMultType ( TiXmlElement * e );
        bool parseSpecialRule ( TiXmlElement * e );
        bool parseSection ( TiXmlElement * e );
        bool parseBand ( TiXmlElement * e );
        bool parseContestSeries ( TiXmlElement * e );
        bool parseContest ( TiXmlElement * e );

        bool parseTimeList ( TiXmlElement * e, QVector<TimeList> &timeList );

        QString version;
        QMap<QString, MultType> mults;
        QMap<QString, SpecialRule> specialRules;
        QMap<QString, CalendarSection> sections;
        QMap<QString, CalendarBand> bands;
        QMap<QString, ContestSeries> contestSeries;
        QMap<QString, CalendarContest> contests;

        QVector<IndividualContest> calendar;

        //      bool findContest(const QString desc, TDateTime start, const BandInfo &bi, IndividualContest &ic);
};

#endif // CALENDAR_H
