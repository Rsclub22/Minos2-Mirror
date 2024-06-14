#ifndef STDINREADER_H
#define STDINREADER_H
#include <QThread>
#include <QMainWindow>

class QLocalServer;
class QLocalSocket;
class StdInReader: public QObject
{
    Q_OBJECT

    void setShowApp(bool /*state*/);

    QMainWindow *qmw = nullptr;

    QLocalServer *localServer =  nullptr;     // The local socket Server
    QLocalSocket *localSocket = nullptr;
    QString appName;            // Service name


public:
    StdInReader(QMainWindow *m);
    ~StdInReader();

private slots:
    void executeStdIn(QString cmd);

    // A trigger new connections
    void newLocalConnection();


    void onReadyRead();
signals:
    void stdinLine(QString);
};

#endif // STDINREADER_H
