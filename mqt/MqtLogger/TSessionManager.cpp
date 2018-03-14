#include "logger_pch.h"

#include "tlogcontainer.h"
#include "enqdlg.h"

#include "TSessionManager.h"
#include "ui_TSessionManager.h"

TSessionManager::TSessionManager(TLogContainer *parent) :
    QDialog(parent), tlc(parent),
    inShowSession(false), inShowSessions(false),
    ui(new Ui::TSessionManager)
{
    ui->setupUi(this);
    QSettings settings;
    QByteArray geometry = settings.value("TSessionManager/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    parseSessions();
    showSessions();
    showSession(sessionList.currentSession);
}
TSessionManager::~TSessionManager()
{
    delete ui;
}


void TSessionManager::enableButtons()
{
    int nsess = sessionList.sessions.count();
    int nentries = 0;
    int currEntry = 0;
    if (nsess)
    {
        nentries = sessionList.sessions[sessionList.currentSession].entries.count();
        currEntry = sessionList.sessions[sessionList.currentSession].currentEntry;
    }

    ui->CloneSessionButton->setEnabled(nsess > 0);
    ui->DeleteSessionButton->setEnabled(nsess > 0);
    ui->RenameSessionButton->setEnabled(nsess > 0);
    ui->AddEntryButton->setEnabled(nsess > 0);
    ui->DeleteEntryButton->setEnabled(nentries > 0);
    ui->MoveEntryUpButton->setEnabled(nentries > 0 && currEntry >= 1);          // not first entry
    ui->MoveEntryDownButton->setEnabled(nentries > 0 && currEntry < nentries - 1);  // not last entry
}
void TSessionManager::parseSessions()
{
    TContestApp *app = TContestApp::getContestApp();
    QStringList sessions = tlc->getSessions();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;

    preloadBundle.startGroup();
    for ( int i = 0; i < sessions.size(); i++ )
    {
        QString sessName = sessions[i];
       if ( sessName == app->currSession )
          sessionList.currentSession = i;
       Session sess;
       sess.sessionName = sessName;
       sessionList.sessions.push_back(sess);

       preloadBundle.openSection(sessName);
       QStringList slotlst = preloadBundle.getProfileEntries();

       if (slotlst.count())
       {
           for ( int j = 0; j < slotlst.count(); j++ )
           {
               QString ent;
               preloadBundle.getStringProfile( slotlst[j], ent, "" );

               if (slotlst[j] == "CurrentLog")
               {
                   sessionList.sessions[i].currentEntry = ent.toInt();
                   continue;
               }
               // get each value
               sessionList.sessions[i].entries.push_back(ent);
           }
       }
    }
    preloadBundle.endGroup();
}
void TSessionManager::writeSessions()
{
    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;
    QStringList sessionlst = app ->logsPreloadBundle.getSections();

    for (int i = 0; i < sessionlst.count(); i++)
    {
        preloadBundle.openSection(sessionlst[i]);
        preloadBundle.clearProfileSection(true);
    }
    preloadBundle.flushProfile();
    preloadBundle.openSection(app->preloadsect);
    if (sessionList.sessions.count())
    {
        preloadBundle.setStringProfile(eppSession, sessionList.sessions[sessionList.currentSession].sessionName);
    }
    preloadBundle.setStringProfile(eppDefSession, app ->defaultSession );

    for (int i = 0; i < sessionList.sessions.count(); i++)
    {
        preloadBundle.openSection(sessionList.sessions[i].sessionName);
        for (int j = 0; j < sessionList.sessions[i].entries.count(); j++)
        {
            preloadBundle.setStringProfile(QString::number(j), sessionList.sessions[i].entries[j]);
        }
        preloadBundle.setIntProfile(eppCurrent, sessionList.sessions[i].currentEntry);
    }
    preloadBundle.flushProfile();
}

void TSessionManager::showSessions()
{
    if (!inShowSessions && !inShowSession)
    {
        inShowSessions = true;
        if (sessionList.currentSession >= sessionList.sessions.count())
        {
            sessionList.currentSession = sessionList.sessions.count() - 1;
        }
        if (sessionList.currentSession < 0)
        {
            sessionList.currentSession = 0;
        }

        ui->sessionsList->clear();
        ui->sessionEntries->clear();

        for ( int i = 0; i < sessionList.sessions.count(); i++ )
        {
           ui->sessionsList->addItem( sessionList.sessions[ i ].sessionName );
        }
        inShowSessions = false;

        showSession(sessionList.currentSession);
        enableButtons();

    }
}

void TSessionManager::showSession(int sess)
{
    if (!inShowSessions && !inShowSession)
    {
        inShowSession = true;
        int rowCount = 0;
        ui->sessionEntries->clear();
        if (sessionList.sessions.count())
        {
            if (sessionList.sessions[sess].currentEntry >= sessionList.sessions[sess].entries.count())
            {
                sessionList.sessions[sess].currentEntry = sessionList.sessions[sess].entries.count() - 1;
            }
            if (sessionList.sessions[sess].currentEntry < 0)
            {
                sessionList.sessions[sess].currentEntry = 0;
            }

            sessionList.currentSession = sess;

            ui->sessionsList->setCurrentRow(sessionList.currentSession);
            rowCount = sessionList.sessions[sess].entries.count();


            ui->sessionEntries->setColumnCount(1);
            ui->sessionEntries->setRowCount(rowCount);

            for ( int i = 0; i < rowCount; i++ )
            {
                ui->sessionEntries->setItem(i, 0, new QTableWidgetItem(sessionList.sessions[sess].entries[i]));
            }
            if (sessionList.sessions[sess].entries.count())
            {
                ui->sessionEntries->selectRow(sessionList.sessions[sess].currentEntry);
            }
        }

        enableButtons();
        inShowSession = false;
    }
}

void TSessionManager::on_OKButton_clicked()
{
    // need to get rewrite the set of sessions
    writeSessions();
    // and go back and select the new current one
    accept();
}


void TSessionManager::on_amendButton_clicked()
{
    writeSessions();
}
void TSessionManager::on_cancelButton_clicked()
{
    // ignore it all... just let our structures die.
    reject();
}
void TSessionManager::doCloseEvent()
{
    QSettings settings;
    settings.setValue("TSessionManager/geometry", saveGeometry());
}
void TSessionManager::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TSessionManager::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void TSessionManager::on_NewSessionButton_clicked()
{
    // create and enter a new session
    QString newName;
    QString prompt = "Give name for new session";
    if (enquireDialog(this, prompt, newName))
    {
        Session newSession;
        newSession.sessionName = newName;
        sessionList.sessions.push_back(newSession);
        sessionList.currentSession = sessionList.sessions.count() - 1;
        showSessions();
    }

}

void TSessionManager::on_CloneSessionButton_clicked()
{
    // copy the current session with  new name, enter it
    QString newName;
    Session newSession = sessionList.sessions[sessionList.currentSession];
    QString prompt = "Give name for new log set";
    if (enquireDialog(this, prompt, newName))
    {
        newSession.sessionName = newName;
        sessionList.sessions.push_back(newSession);
        sessionList.currentSession = sessionList.sessions.count() - 1;
        showSessions();
    }
}

void TSessionManager::on_DeleteSessionButton_clicked()
{
    // delete the current session, move on one
    sessionList.sessions.remove(sessionList.currentSession);
    showSessions();
}

void TSessionManager::on_AddEntryButton_clicked()
{
    // add an entry to the current session beow the current entry
    //- need to put up a file selection dialog
    // AND we want it to be multiselect

    QString InitialDir = tlc->getDefaultDirectory( false );

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = "Minos contest files (*.minos *.Minos);;"
                     "All Files (*.*)" ;

    QStringList fnames = QFileDialog::getOpenFileNames( this,
                       "Open contests",
                       InitialDir,  // dir
                       Filter
                       );
    for (int i = 0; i < fnames.size(); i++)
    {
        QString fname = fnames[i];
        if ( !fname.isEmpty() )
        {
            sessionList.sessions[sessionList.currentSession].entries.push_back(fname);
        }
    }

    showSession(sessionList.currentSession);
}

void TSessionManager::on_DeleteEntryButton_clicked()
{
    // delete the current entry from the current session
    int &ce = sessionList.sessions[sessionList.currentSession].currentEntry;
    QVector<QString> &entries = sessionList.sessions[sessionList.currentSession].entries;

    entries.remove(ce);
    showSession(sessionList.currentSession);
}

void TSessionManager::on_MoveEntryUpButton_clicked()
{
    // shift the current entry up one
    int &ce = sessionList.sessions[sessionList.currentSession].currentEntry;
    QVector<QString> &entries = sessionList.sessions[sessionList.currentSession].entries;

    std::swap(entries[ce], entries[ce -1]);
    ce--;

    showSession(sessionList.currentSession);
}

void TSessionManager::on_MoveEntryDownButton_clicked()
{
    // shift the current entry down one
    int &ce = sessionList.sessions[sessionList.currentSession].currentEntry;
    QVector<QString> &entries = sessionList.sessions[sessionList.currentSession].entries;

    std::swap(entries[ce], entries[ce +1]);
    ce++;

    showSession(sessionList.currentSession);
}

void TSessionManager::on_RenameSessionButton_clicked()
{
    // change the session name
    QString newName;
    QString prompt = "Give new name for log set " + sessionList.sessions[sessionList.currentSession].sessionName;
    if (enquireDialog(this, prompt, newName))
    {
        sessionList.sessions[sessionList.currentSession].sessionName = newName;
        showSessions();
    }
}

void TSessionManager::on_sessionsList_currentRowChanged(int currentRow)
{
    if ( !inShowSession && !inShowSession )
    {
        showSession(currentRow);
    }
}

void TSessionManager::on_sessionEntries_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem * /*previous*/)
{
    if ( !inShowSession && !inShowSession )
    {
        if (current)
        {
            sessionList.sessions[sessionList.currentSession].currentEntry = current->row();
        }
        enableButtons();
    }
}
