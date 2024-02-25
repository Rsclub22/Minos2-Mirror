#include "regsettings.h"
#include "MTrace.h"
#include "MMessageDialog.h"
#include "MShowMessageDlg.h"
#include "fileutils.h"
#include "tlogcontainer.h"
#include <QSslSocket>
#include "tcalendarform.h"
#include "ui_tcalendarform.h"


TCalendarForm::
TCalendarForm( QWidget *parent, CalType calType ) :
        QDialog( parent )
      , ui( new Ui::TCalendarForm )
      , calType ( calType )
      , vhf ( 2000, ectVHF )
      , hf ( 2000, ectHF )
      , mwave ( 2000, ectMwave )
      , hfother ( 2000, ectHFOther )
      , vhfother ( 2000, ectVHFOther )
      , hfbartg( 2000, ectHFBARTG )
      , uksmg( 2000, ectUKSMG )
{
    ui->setupUi( this );
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}
int TCalendarForm::exec()
{
    ui->YearEdit->setText( QDate::currentDate().toString ( "yyyy" ) );

    FormShow();

    RegSettings settings;
    QString sname = "Calendar/grid_geometry_" + calendarNameString[calType];
    QByteArray state = settings.getSettings().value(sname).toByteArray();
    if (state.size())
    {
        ui->CalendarGrid->horizontalHeader()->restoreState(state);
    }
    ui->CalendarGrid->horizontalHeader()->setStretchLastSection(true);

    QByteArray geometry = settings.getSettings().value("Calendar/form_geometry_" + calendarNameString[calType]).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    return QDialog::exec();
}

TCalendarForm::~TCalendarForm()
{
    delete ui;
}
void TCalendarForm::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TCalendarForm::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void TCalendarForm::doCloseEvent()
{
    RegSettings settings;

    settings.getSettings().setValue("Calendar/grid_geometry_" + calendarNameString[calType], ui->CalendarGrid->horizontalHeader()->saveState());

    settings.getSettings().setValue("Calendar/form_geometry_" + calendarNameString[calType], saveGeometry());
}

bool TCalendarForm::loadYear ( Calendar &cal, int year )
{
    int yearOffset = -1;
    for ( int i = yearList.size() - 1; i >= 0; i-- )
    {
        if (yearList[ i ] ->yearOffset  == 0)
        {
             yearOffset = i;
        }
    }
    QString fname = yearList[ yearOffset ] ->getPath();
    bool thisYearExists = FileExists ( fname );

    if (!thisYearExists && cal.calType != ectUKSMG)
    {
        QString yearMessage = tr("This year's calendar file\n\n%1\n\ndoes not exist; do you wish to download the latest calendars?").arg(fname);
        if (mShowYesNoMessage(this, yearMessage))
        {
            downloadFiles();
        }
    }

    bool loaded = false;

    for ( int i = yearList.size() - 1; i >= 0; i-- )
    {
        if ( !loaded && FileExists ( yearList[ i ] ->getPath() ) && year >= calendarFormYear + yearList[ i ] ->yearOffset )
        {
            loaded = cal.parseFile ( yearList[ i ] ->getPath() );
        }
    }
    return loaded;
}
//---------------------------------------------------------------------------
void TCalendarForm::LoadGrid ( Calendar &cal )
{
    ui->CalendarVersionLabel->setText( ( "File Version " + cal.version ) );
    ui->CalendarGrid->setRowCount( cal.calendar.size() + 1 );
    int cc = ( cal.calType == ectVHF ? 8 : 5 );

    ui->CalendarGrid->setColumnCount( cc );
    int col = 0;
    ui->CalendarGrid->setHorizontalHeaderItem( col++, new QTableWidgetItem( tr("Contest Name") ) );
    ui->CalendarGrid->setHorizontalHeaderItem( col++, new QTableWidgetItem( tr("Band") ) );
    ui->CalendarGrid->setHorizontalHeaderItem( col++, new QTableWidgetItem( tr("Start (UTC)") ) );
    ui->CalendarGrid->setHorizontalHeaderItem( col++, new QTableWidgetItem( tr("Finish (UTC)") ) );
    if ( cal.calType == ectVHF )
    {
        ui->CalendarGrid->setHorizontalHeaderItem( col++, new QTableWidgetItem( tr("Scoring") ) );
        ui->CalendarGrid->setHorizontalHeaderItem( col++, new QTableWidgetItem( tr("Mults") ) );
    }
    ui->CalendarGrid->setHorizontalHeaderItem( col++, new QTableWidgetItem( tr("Sections") ) );
    if ( cal.calType == ectVHF )
    {
        ui->CalendarGrid->setHorizontalHeaderItem( col++, new QTableWidgetItem( tr("Special Rules") ) );
    }
    int row = 0;
    int nextContest = -1;
    QDateTime now = QDateTime::currentDateTime();
    for ( auto const &c: qAsConst(cal.calendar ))
    {
        col = 0;
        ui->CalendarGrid->setItem( row, col++, new QTableWidgetItem( c.description ) );
        ui->CalendarGrid->setItem( row, col++, new QTableWidgetItem( c.bands ) );
        ui->CalendarGrid->setItem( row, col++, new QTableWidgetItem( c.start.toString ( "dd/MM/yyyy HH:mm" ) ) );
        ui->CalendarGrid->setItem( row, col++, new QTableWidgetItem( c.finish.toString ( "dd/MM/yyyy HH:mm" ) ) );

        if ( cal.calType == ectVHF )
        {
            ui->CalendarGrid->setItem( row, col++, new QTableWidgetItem( c.ppKmScoring ? tr("1Pt/Km") : tr("1Pt/QSO") ) );
            ui->CalendarGrid->setItem( row, col++, new QTableWidgetItem( c.mults ) );
        }
        ui->CalendarGrid->setItem( row, col++, new QTableWidgetItem( c.sections ) );
        if ( cal.calType == ectVHF )
        {
            ui->CalendarGrid->setItem( row, col++, new QTableWidgetItem( c.specialRules ) );
        }

        if (!description.isEmpty())
         {
            if (nextContest < 0 && description == c.description)
            {
                if ((c.start == sdate) && (c.reg1band == band || c.ukband == band))
                    nextContest = row;
            }
         }
         else
         {
            if (nextContest < 0 && (c.start.daysTo(c.finish) < 360) && now <= c.finish)
            {
               // don't select the test contests
               nextContest = row;
            }
         }
        row++;
    }
    if (nextContest >= 0)
    {
        ui->CalendarGrid->selectRow(nextContest);
    }
}
void TCalendarForm::FormShow ( )
{

    bool ok;
    int year = ui->YearEdit->text().toInt(&ok);
    if (!ok)
        year = 2000;

    bool loaded = false;
    if ( calType == ectHF )
    {
        setWindowTitle ( "Select Contest from HF Calendar" );
        hf = Calendar ( year, ectHF );

        yearList.clear();
        for ( int i = LOWYEAR; i <= HIGHYEAR; i++ )
        {
            yearList.push_back ( QSharedPointer<CalendarYear> ( new HFCalendarYear ( i ) ) );
        }

        loaded = loadYear ( hf, year );

        if ( !loaded )
        {
            mShowMessage ( tr("Failed to load the HF calendar file"), this );
            ui->CalendarVersionLabel->setText( tr("No file loaded") );
            ui->CalendarGrid->setRowCount( 0 );
            return ; // don't close - they need a chance to download
        }
        LoadGrid ( hf );
    }
    else
        if ( calType == ectHFBARTG )
        {
            setWindowTitle (tr("Select Contest from BARTG Calendar") );
            hfbartg = Calendar ( year, ectHFBARTG );

            yearList.clear();
            for ( int i = LOWYEAR; i <= HIGHYEAR; i++ )
            {
                yearList.push_back ( QSharedPointer<CalendarYear> ( new HFBARTGCalendarYear ( i ) ) );
            }

            loaded = loadYear ( hfbartg, year );

            if ( !loaded )
            {
                mShowMessage ( tr("Failed to load the BARTG calendar file"), this );
                ui->CalendarVersionLabel->setText( tr("No file loaded") );
                ui->CalendarGrid->setRowCount( 0 );
                return ; // don't close - they need a chance to download
            }
            LoadGrid ( hfbartg );
        }
        else
            if ( calType == ectVHF )
            {
                setWindowTitle ( tr("Select Contest from VHF Calendar") );
                vhf = Calendar ( year, ectVHF );

                yearList.clear();
                for ( int i = LOWYEAR; i <= HIGHYEAR; i++ )
                {
                    yearList.push_back ( QSharedPointer<CalendarYear> ( new VHFCalendarYear ( i ) ) );
                }

                loaded = loadYear ( vhf, year );

                if ( !loaded )
                {
                    mShowMessage ( tr("Failed to load the VHF calendar file"), this );
                    ui->CalendarVersionLabel->setText( tr("No file loaded") );
                    ui->CalendarGrid->setRowCount( 0 );
                    return ; // don't close - they need a chance to download
                }
                LoadGrid ( vhf );
            }
            else
                if ( calType == ectHFOther )
                {
                    setWindowTitle ( tr("Select Contest from HF other Calendar") );
                    hfother = Calendar ( year, ectHFOther );
                    yearList.clear();
                    for ( int i = LOWYEAR; i <= HIGHYEAR; i++ )
                    {
                        yearList.push_back ( QSharedPointer<CalendarYear> ( new HFOtherCalendarYear ( i ) ) );
                    }

                    loaded = loadYear ( hfother, year );

                    if ( !loaded )
                    {
                        mShowMessage ( tr("Failed to load the HF other calendar file"), this );
                        ui->CalendarVersionLabel->setText(tr( "No file loaded") );
                        ui->CalendarGrid->setRowCount( 0 );
                        return ; // don't close - they need a chance to download
                    }
                    LoadGrid ( hfother );
                }
                else
                    if ( calType == ectVHFOther )
                    {
                        setWindowTitle ( tr("Select Contest from VHF other Calendar") );
                        vhfother = Calendar ( year, ectVHFOther );

                        yearList.clear();
                        for ( int i = LOWYEAR; i <= HIGHYEAR; i++ )
                        {
                            yearList.push_back ( QSharedPointer<CalendarYear> ( new VHFOtherCalendarYear ( i ) ) );
                        }

                        loaded = loadYear ( vhfother, year );
                        if ( !loaded )
                        {
                            mShowMessage ( tr("Failed to load the VHF other calendar file"), this );
                            ui->CalendarVersionLabel->setText( tr("No file loaded") );
                            ui->CalendarGrid->setRowCount( 0 );
                            return ; // don't close - they need a chance to download
                        }
                        LoadGrid ( vhfother );
                    }
                    else
                        if ( calType == ectMwave )
                        {
                            setWindowTitle ( tr("Select Contest from Microwave Calendar") );
                            mwave = Calendar ( year, ectMwave );

                            yearList.clear();
                            for ( int i = LOWYEAR; i <= HIGHYEAR; i++ )
                            {
                                yearList.push_back ( QSharedPointer<CalendarYear> ( new MicroCalendarYear ( i ) ) );
                            }

                            loaded = loadYear ( mwave, year );

                            if ( !loaded )
                            {
                                mShowMessage ( tr("Failed to load the Microwave calendar file"), this );
                                ui->CalendarVersionLabel->setText( tr("No file loaded") );
                                ui->CalendarGrid->setRowCount( 0 );
                                return ; // don't close - they need a chance to download
                            }
                            LoadGrid ( mwave );
                        }
                        else
                            if ( calType == ectUKSMG )
                            {
                                setWindowTitle ( tr("Select Contest from UKSMG Calendar") );
                                uksmg = Calendar ( year, ectUKSMG );

                                yearList.clear();
                                for ( int i = LOWYEAR; i <= HIGHYEAR; i++ )
                                {
                                    yearList.push_back ( QSharedPointer<CalendarYear> ( new UKSMGCalendarYear ( i ) ) );
                                }

                                loaded = loadYear ( uksmg, year );
                                if ( !loaded )
                                {
                                    mShowMessage ( tr("Failed to load the UKSMG calendar file"), this );
                                    ui->CalendarVersionLabel->setText( tr("No file loaded") );
                                    ui->CalendarGrid->setRowCount( 0 );
                                    return ; // don't close - they need a chance to download
                                }
                                LoadGrid ( uksmg );
                            }
}
void TCalendarForm::downloadFiles()
{
    trace(QString("OpenSSL version build is %1").arg(QSslSocket::sslLibraryBuildVersionString()));
    if (!QSslSocket::supportsSsl())
    {
        // NB MSVC 2015 redistributable may be needed for OpenSSL
        mShowMessage(tr("Something is wrong - SSL not supported on this system.")
                     , this);

        return;
    }

    int fileCount = 0;

    QVector<QSharedPointer<CalendarYear> > yearList;

    yearList.push_back ( QSharedPointer<CalendarYear> ( new CTYCalendarYear ( 0 ) ) );
    yearList.push_back ( QSharedPointer<CalendarYear> ( new ClubListCalendarYear ( 0 ) ) );
    for ( int i = LOWURLYEAR; i <= HIGHYEAR; i++ )
    {
        yearList.push_back ( QSharedPointer<CalendarYear> ( new VHFCalendarYear ( i ) ) );
        yearList.push_back ( QSharedPointer<CalendarYear> ( new HFCalendarYear ( i ) ) );
        yearList.push_back ( QSharedPointer<CalendarYear> ( new HFBARTGCalendarYear ( i ) ) );
        yearList.push_back ( QSharedPointer<CalendarYear> ( new MicroCalendarYear ( i ) ) );
        yearList.push_back ( QSharedPointer<CalendarYear> ( new UKSMGCalendarYear ( i ) ) );
    }

    for ( auto const &y: yearList )
    {
        if ( y->downloadFile ( false, LogContainer ) )
        {
            fileCount++;
        }
    }

    mShowMessage( tr("%1 of %2 files downloaded. We don't expect to load them all.").arg(fileCount).arg(yearList.size()), this);
}
//---------------------------------------------------------------------------

void TCalendarForm::on_CloseButton_clicked()
{
    reject();
}

void TCalendarForm::on_SelectButton_clicked()
{
    int row = ui->CalendarGrid->currentRow();
    if ( calType == ectHF )
    {
        if ( row >= 0 && row < hf.calendar.size())
        {
            ic = hf.calendar[ row ];
            accept();
        }
    }
    else
        if ( calType == ectHFOther )
        {
            if ( row >= 0 && row < hfother.calendar.size())
            {
                ic = hfother.calendar[ row ];
                accept();
            }
        }
        else
            if ( calType == ectVHF )
            {
                if ( row >= 0 && row < vhf.calendar.size())
                {
                    ic = vhf.calendar[ row ];
                    accept();
                }
            }
            else
                if ( calType == ectVHFOther )
                {
                    if ( row >= 0 && row < vhfother.calendar.size())
                    {
                        ic = vhfother.calendar[ row ];
                        accept();
                    }
                }
                else
                    if ( calType == ectMwave )
                    {
                        if ( row >= 0 && row < mwave.calendar.size())
                        {
                            ic = mwave.calendar[ row ];
                            accept();
                        }

                    }
                    else
                        if ( calType == ectHFBARTG )
                        {
                            if ( row >= 0 && row < hfbartg.calendar.size())
                            {
                                ic = hfbartg.calendar[ row ];
                                accept();
                            }

                        }
                        else
                            if ( calType == ectUKSMG )
                            {
                                if ( row >= 0 && row < uksmg.calendar.size())
                                {
                                    ic = uksmg.calendar[ row ];
                                    accept();
                                }

                            }
}

void TCalendarForm::on_GetCalendarButton_clicked()
{
    downloadFiles();
    FormShow ( );
}

void TCalendarForm::on_YearDownButton_clicked()
{
    bool ok;
    int year = ui->YearEdit->text().toInt(&ok);
    if (!ok)
        year = calendarFormYear;

    year--;

    if ( year < calendarFormYear + LOWYEAR )
    {
        year = calendarFormYear + HIGHYEAR;
    }
    ui->YearEdit->setText( QString::number( year ) );

    FormShow ( );

}

void TCalendarForm::on_YearUpButton_clicked()
{
    bool ok;
    int year = ui->YearEdit->text().toInt(&ok);
    if (!ok)
        year = calendarFormYear;
    year++;

    if ( year > calendarFormYear + HIGHYEAR )
    {
        year = calendarFormYear + LOWYEAR;
    }
    ui->YearEdit->setText( QString::number( year ) );

    FormShow ( );
}

void TCalendarForm::on_CalendarGrid_doubleClicked(const QModelIndex &/*index*/)
{
    on_SelectButton_clicked();
}

