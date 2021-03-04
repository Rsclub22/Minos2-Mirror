#ifndef STDINREADER_H
#define STDINREADER_H
#include <QThread>

bool getShowApp( );
void setShowApp(bool /*state*/);

class StdInReader: public QThread
{
    Q_OBJECT

    virtual void run();
public:
    StdInReader();
    ~StdInReader();

signals:
    void stdinLine(QString);
};

#endif // STDINREADER_H
