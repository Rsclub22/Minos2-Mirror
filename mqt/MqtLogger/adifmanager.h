#ifndef ADIFMANAGER_H
#define ADIFMANAGER_H

#include <QObject>
#include <QTimer>

class LoggerContestLog;

class AdifManager: public QObject
{
    Q_OBJECT
public:
    AdifManager(LoggerContestLog * const c, QString fname, qint64 lo);
    virtual ~AdifManager() override;
    QString getWatchedFile() const;

private:
    QTimer fileTimer;
    QString watchedFile;
    qint64 lastOffset = 0;
    LoggerContestLog * ct = nullptr;
    void AdifFileChanged();
private slots:
    void checkFile();
signals:
    void adifImportFailed();

};

#endif // ADIFMANAGER_H
