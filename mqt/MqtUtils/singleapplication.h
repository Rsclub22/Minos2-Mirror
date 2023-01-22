#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QObject>
#include <QApplication>
#include <QLocalServer>
#include <QWidget>

class SingleApplication : public QApplication {
        Q_OBJECT
    public:
        SingleApplication(QString appName, int &argc, char **argv);

        bool isRunning();                // Are there instances running
        void sendArgs();
        void sendPid(QLocalSocket *socket);
        QString getPid(){return pid;}
        static bool testRunning(QString name, int timeout, QString &pid);

    private slots:
        // A trigger new connections
        void _newLocalConnection();

    private:
        // Initialize the local connection
        void _initLocalConnection();
        // Create the app server
        void _newLocalServer();

        bool _isRunning;                // Are there instances running
        QLocalServer *_localServer;     // The local socket Server
        QString _appName;            // Service name

        QString pid;

    signals:
        void argsReceived(QString);

    public slots:
        void clearRegistry();
};

#endif // SINGLEAPPLICATION_H
