#ifndef CalendarListH
#define CalendarListH 
//---------------------------------------------------------------------------
#include "base_pch.h"
#include "Calendar.h"
extern int calendarFormYear;
#define LOWYEAR -1
#define LOWURLYEAR -1
#define HIGHYEAR 1

extern QString calendarNameString[];

class CalendarYear
{
        virtual QString getSite() = 0;
        QString yearString()
        {
            QString y = QString::number( calendarFormYear + yearOffset );
            y = y.mid( 2, 2 );
            return y;
        }
    public:
        CalendarYear ( CalType t, int y ) :
            yearOffset ( y )
            , type ( t )
        {
            if ( calendarFormYear == 0 )
            {
                calendarFormYear = QDate::currentDate().year();
            }

        }
        virtual ~CalendarYear(){}

        virtual bool downloadFile ( bool showError, QWidget *parent );

        bool loaded;
        int yearOffset;
        CalType type;
        virtual QString getPath();
        virtual QString getURL();
};
class HFCalendarYear : public CalendarYear
{
        virtual QString getSite();
    public:
        HFCalendarYear ( int year ) : CalendarYear ( ectHF, year )
        {
        }
};
class HFBARTGCalendarYear : public CalendarYear
{
        virtual QString getSite();
    public:
        HFBARTGCalendarYear ( int year ) : CalendarYear ( ectHFBARTG, year )
        {
        }
};
class VHFCalendarYear : public CalendarYear
{
        virtual QString getSite();
    public:
        VHFCalendarYear ( int year ) : CalendarYear ( ectVHF, year )
        {
        }
};
class MicroCalendarYear : public CalendarYear
{
        virtual QString getSite();
    public:
        MicroCalendarYear ( int year ) : CalendarYear ( ectMwave, year )
        {
        }
};
class HFOtherCalendarYear : public CalendarYear
{
        virtual QString getSite();
    public:
        HFOtherCalendarYear ( int year ) : CalendarYear ( ectHFOther, year )
        {
        }
        virtual QString getPath();
        virtual QString getURL();
};
class VHFOtherCalendarYear : public CalendarYear
{
        virtual QString getSite();
    public:
        VHFOtherCalendarYear ( int year ) : CalendarYear ( ectVHFOther, year )
        {
        }
        virtual QString getPath();
        virtual QString getURL();
};
class CTYCalendarYear : public CalendarYear
{
        virtual QString getSite();
    public:
        CTYCalendarYear ( int year ) : CalendarYear ( ectVHFOther, year )
        {
        }
        virtual QString getPath();
        virtual QString getURL();
};

#endif
