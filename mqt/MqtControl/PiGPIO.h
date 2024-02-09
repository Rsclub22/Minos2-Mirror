#ifndef PIGPIO_H
#define PIGPIO_H

#include <QSocketNotifier>
#include <QMap>

class PiGPIO;

class GPIOLine: public QObject
{
    Q_OBJECT
    bool input;
    int pin;
    int fd;
public:
    GPIOLine(int pin, bool input);
    virtual ~GPIOLine() override;

    bool initialise();
    void setPin(bool state);
    bool readPin();

};

class PiGPIO: public QObject
{
    Q_OBJECT

    QMap<int, QSharedPointer<GPIOLine> > exportedPins;
public:
    PiGPIO();
    virtual ~PiGPIO() override;
    bool setPinInput(int pin);
    bool setPinOutput(int pin);

    void setPin(int pin, bool state);
    bool readPin(int pin);

};

#endif // PIGPIO_H
