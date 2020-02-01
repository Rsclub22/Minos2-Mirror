#ifndef ROTATORBASE_H
#define ROTATORBASE_H

#include <QObject>

class RotatorBase : public QObject
{
    Q_OBJECT
public:
    explicit RotatorBase(QObject *parent = nullptr);

    void setRotAzimuth(int rot_azimuth_){rot_azimuth = rot_azimuth_;}
    int getRotAzimuth(){return rot_azimuth;}

    void setRotConnected(bool rotConnected_){rotConnected = rotConnected_;}
    bool getRotConnected(){return rotConnected;}

signals:


protected:

    virtual int request_bearing() = 0;
    virtual int rotate_to_bearing(int bearing) = 0;
    virtual int rotateCClockwise(int speed) = 0;
    virtual int rotateClockwise(int speed) = 0;
    virtual int stop_rotation() = 0;

private:

    int rot_azimuth;
    bool rotConnected;


};

#endif // ROTATORBASE_H
