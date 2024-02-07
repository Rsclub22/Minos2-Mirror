#ifndef ADIFMANAGER_H
#define ADIFMANAGER_H

#include <QObject>

class LoggerContestLog;
class QFileSystemWatcher;

class AdifManager: public QObject
{
    Q_OBJECT
public:
    AdifManager(LoggerContestLog * const c, QString fname, qint64 lo);
    virtual ~AdifManager() override;
    QString getWatchedFile() const;

private:
    QFileSystemWatcher *adifWatcher = nullptr;
    QString watchedFile;
    qint64 lastOffset = 0;
    LoggerContestLog * ct = nullptr;
private slots:
    void AdifFileChanged();
signals:
    void adifImportFailed();

};

#endif // ADIFMANAGER_H
