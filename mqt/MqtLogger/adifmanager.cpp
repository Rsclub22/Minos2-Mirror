#include <QFileInfo>

#include "fileutils.h"
#include "tlogcontainer.h"
#include "LoggerContest.h"
#include "AdifImport.h"
#include "MinosLoggerEvents.h"
#include "tsinglelogframe.h"
#include "adifmanager.h"

/*
    I'd use QFileWatcher, but it doesn't seem to work when records are dripped
    into the file with no file detail change
*/
AdifManager::AdifManager(LoggerContestLog * const c, QString fname, qint64 lo)
    :ct(c), watchedFile(fname), lastOffset(lo)
{
    if (lo < 0)
    {
        QFile wf(watchedFile, this);
        bool ok = wf.open(QIODevice::ReadOnly);
        if (ok)
        {
            lastOffset = wf.size();
        }
        else
        {
            lastOffset = 0;
        }
    }
    ct->watchedADIFLastOffset.setValue(lastOffset);
    ct->watchedADIFFile.setValue(watchedFile);
    ct->commonSave(false);

    connect(&fileTimer, &QTimer::timeout, this, &AdifManager::checkFile);

    if (!FileExists(watchedFile))
    {
        ct->watchedADIFLastOffset.setValue(0);
    }
    fileTimer.start(1000);

}
AdifManager::~AdifManager()
{
    fileTimer.stop();
}
void AdifManager::checkFile()
{
    QString watchedFile = ct->watchedADIFFile.getValue();
    if (FileExists(watchedFile))
    {
        AdifFileChanged();
    }

}
QString AdifManager::getWatchedFile() const
{
    return watchedFile;
}
void AdifManager::AdifFileChanged()
{
    QFile wf(watchedFile, this);
    bool ok = wf.open(QIODevice::ReadOnly);
    qint64 fsz = 0;

    if (ok)
    {
        fsz = wf.size();
        if (lastOffset >= fsz)
        {
            lastOffset = fsz;
            ok = false;
        }
        if (lastOffset < 0)
        {
            lastOffset = 0;
        }
    }

    while (ok && lastOffset < fsz)
    {
        ok = wf.seek(lastOffset);
        if (ok)
        {
            QByteArray all = QByteArray("Header\n<EOH>") + wf.readLine();
            if (all.size())
            {
                ok = ADIFImport::doImportADIFString(ct, all);
                lastOffset = wf.pos();
                ct->watchedADIFLastOffset.setValue(lastOffset);
            }
            else
            {
                // who connects to this, and what then happens?
                emit adifImportFailed();
            }
        }
        else
        {
            // who connects to this, and what then happens?
            emit adifImportFailed();
        }
    }
    if (ok)
    {
        // this is common with append? But in different class
        // WsjtxFrame has similar

        QSharedPointer<BaseContact> bct;
        int spoint = ct->ctList.count();
        for ( int i = spoint; i < ct->ctList.count(); i++ )
        {
            // do we ever get multiple QSOs in one ADIF? We can here.
            // not at least as sent from WSJT-X et al!
            bct = ct->pcontactAt(i);
            bct->commonSave(bct);
        }
        ct->commonSave( false );
        ct->scanContest();
        for ( int i = spoint; i != ct->ctList.count(); i++ )
        {
            QSharedPointer<BaseContact> bct = ct->pcontactAt(i);
            MinosLoggerEvents::SendAfterLogContact(ct, bct); // after ADIF logged "last contact"
        }

        TSingleLogFrame * tslf = LogContainer ->findContest( ct );

        tslf->updateTrees();
        tslf->startNextEntry();
    }
}
