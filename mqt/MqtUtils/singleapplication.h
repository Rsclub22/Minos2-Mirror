#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QObject>
#include <QApplication>
#include <QLocalServer>
#include <QWidget>

class SingleApplication : public QApplication {
        Q_OBJECT
    public:
        SingleApplication(QString serverName, int &argc, char **argv);

        bool isRunning();                // Are there instances running
        void sendArgs();

    private slots:
        // A trigger new connections
        void _newLocalConnection();

    private:
        // Initialize the local connection
        void _initLocalConnection();
        // Create the server
        void _newLocalServer();

        bool _isRunning;                // Are there instances running
        QLocalServer *_localServer;     // The local socket Server
        QString _serverName;            // Service name

    signals:
        void argsReceived(QString);

private slots:
        void clearRegistry();
};

#endif // SINGLEAPPLICATION_H
