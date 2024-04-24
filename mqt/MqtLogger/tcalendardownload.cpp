#include <QSslSocket>
//#include "regsettings.h"
#include "CalendarList.h"
#include "MTrace.h"
//#include "MMessageDialog.h"
#include "MShowMessageDlg.h"
//#include "fileutils.h"

#include "tcalendardownload.h"
#include "tlogcontainer.h"
#include "ui_tcalendardownload.h"

TCalendarDownload::TCalendarDownload(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TCalendarDownload)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint));

    formShowTimer.setSingleShot(true);
    connect(&formShowTimer, &QTimer::timeout, this, &TCalendarDownload::on_formShown);
    formShowTimer.start(100);

}

TCalendarDownload::~TCalendarDownload()
{
    delete ui;
}
void TCalendarDownload::on_formShown()
{
    downloadFiles();
    close();
}
void TCalendarDownload::downloadFiles()
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
        ui->dlFileLabel->setText(y->getFileName());
        if ( y->downloadFile ( false, LogContainer ) )
        {
            fileCount++;
        }
    }

    mShowMessage( tr("%1 of %2 files downloaded. We don't expect to load them all.").arg(fileCount).arg(yearList.size()), this);
}
