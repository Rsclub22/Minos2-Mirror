#ifndef COMMANDREADER_H
#define COMMANDREADER_H
#include <QThread>
#include <QMainWindow>

class QLocalServer;
class QLocalSocket;
class CommandReader: public QObject
{
    Q_OBJECT

    void setShowApp(bool /*state*/);

    QMainWindow *qmw = nullptr;

    QLocalServer *localServer =  nullptr;     // The local socket Server
    QLocalSocket *localSocket = nullptr;
    QString appName;            // Service name


public:
    CommandReader(QMainWindow *m);
    ~CommandReader();

private slots:
    void executeCommand(QString cmd);

    // A trigger new connections
    void newLocalConnection();


    void onReadyRead();
signals:
    void commandLine(QString);
};

#endif // COMMANDREADER_H
