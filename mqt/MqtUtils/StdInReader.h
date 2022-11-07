#ifndef STDINREADER_H
#define STDINREADER_H
#include <QThread>
#include <QMainWindow>

class StdInReader: public QThread
{
    Q_OBJECT

    void setShowApp(bool /*state*/);

    virtual void run();

    QMainWindow *qmw = nullptr;

public:
    StdInReader(QMainWindow *m);
    ~StdInReader();

private slots:
    void executeStdIn(QString cmd);

signals:
    void stdinLine(QString);
};

#endif // STDINREADER_H
