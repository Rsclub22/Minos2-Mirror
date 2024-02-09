#include <QFileInfo>
#include <QFileSystemWatcher>

#include "tlogcontainer.h"
#include "LoggerContest.h"
#include "AdifImport.h"
#include "MinosLoggerEvents.h"
#include "tsinglelogframe.h"
#include "adifmanager.h"

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
            ct->watchedADIFLastOffset.setValue(lastOffset);
            ct->watchedADIFFile.setValue(fname);
        }
    }
    adifWatcher = new QFileSystemWatcher(this);
    adifWatcher->addPath(watchedFile);
    connect(adifWatcher, &QFileSystemWatcher::fileChanged, this, &AdifManager::AdifFileChanged);

}
AdifManager::~AdifManager()
{

}

QString AdifManager::getWatchedFile() const
{
    return watchedFile;
}
void AdifManager::AdifFileChanged()
{
    QFile wf(watchedFile, this);
    bool ok = wf.open(QIODevice::ReadOnly);
    int spoint = ct->ctList.count();

    while (ok && lastOffset < wf.size())
    {
        ok = wf.seek(lastOffset);
        if (ok)
        {
            QByteArray all = wf.readLine();
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
