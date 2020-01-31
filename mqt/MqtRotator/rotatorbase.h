#ifndef ROTATORBASE_H
#define ROTATORBASE_H

#include <QObject>

class RotatorBase : public QObject
{
    Q_OBJECT
public:
    explicit RotatorBase(QObject *parent = nullptr);

signals:


protected:

    virtual int request_bearing() = 0;
    virtual int rotate_to_bearing(int bearing) = 0;
    virtual int rotateCClockwise(int speed) = 0;
    virtual int rotateClockwise(int speed) = 0;
    virtual int stop_rotation() = 0;



};

#endif // ROTATORBASE_H
