#include "base_pch.h"
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef Q_OS_WIN
#include <io.h>
#endif

#include "PiGPIO.h"

GPIOLine::GPIOLine(int pin, bool input):pin(pin), input(input), fd(-1)
{

}

GPIOLine::~GPIOLine()
{
    close(fd);
    QString spin = "/sys/class/gpio/unexport";

    fd = open(spin.toLatin1(), O_WRONLY);
    if (-1 == fd)
    {
        trace("Failed to open " + spin + " for writing!");
        return;
    }

    spin = QString::number(pin);
    QByteArray apin = spin.toUtf8();
    size_t written = write(fd, apin.data(), apin.length());
    if (written != static_cast<unsigned int>(apin.length()))
    {
        trace("GPIO destructor: short write");
    }
    close(fd);
}
bool GPIOLine::initialise()
{
    QString spin = "/sys/class/gpio/export";

    fd = open(spin.toLatin1(), O_WRONLY);
    if (-1 == fd)
    {
        trace("Failed to open " + spin + " for writing!");
        return false;
    }

    spin = QString::number(pin);
    QByteArray apin = spin.toUtf8();
    size_t written = write(fd, apin.data(), apin.length());
    if (written != static_cast<unsigned int>(apin.length()))
    {
        trace("GPIO initialise: short write");
    }
    close(fd);

    QThread::msleep(100);

    spin = "/sys/class/gpio/gpio" + spin;
    QString sspin = spin + "/direction";
    fd = open(sspin.toLatin1(), O_WRONLY);
    if (-1 == fd)
    {
        trace("Failed to open" + sspin  + " for writing! errno is " + QString::number(errno));
        return false;
    }

    sspin = input?"in":"out";
    apin = sspin.toUtf8();
    if (-1 == write(fd, apin.data(), apin.length()))
    {
        trace("Failed to set direction!");
        return false;
    }

    close(fd);

    QThread::msleep(100);

    fd = open((spin + "/value").toLatin1(), O_RDWR);
    if (-1 == fd)
    {
        trace("Failed to open " + spin + "/value for read/write! errno is " + QString::number(errno));
        return false;
    }

    QThread::msleep(100);

    return true;
}
void GPIOLine::setPin(bool state)
{
    lseek(fd, 0, SEEK_SET);

    if (1 != write(fd, state ? "1" : "0", 1))
    {
        trace("Failed to write value pin " + QString::number(pin));
    }
}

bool GPIOLine::readPin()
{
    char value_str[4];

    lseek(fd, 0, SEEK_SET);
    int ret = read(fd, value_str, 3);

    if (-1 == ret)
    {
        trace("Failed to read value for pin " + QString::number(pin));
        return false;
    }
    value_str[ret] = 0;
    return value_str[0] == '0';
}


PiGPIO::PiGPIO()
{

}
PiGPIO::~PiGPIO()
{
    exportedPins.clear();
}

void PiGPIO::setPinInput(int pin)
{
    QSharedPointer<GPIOLine> line(new GPIOLine(pin, true));
    if (line->initialise())
    {
        exportedPins[pin] = line;
    }
}

void PiGPIO::setPinOutput(int pin)
{
    QSharedPointer<GPIOLine> line(new GPIOLine(pin, false));
    if (line->initialise())
    {
        exportedPins[pin] = line;
    }
}

void PiGPIO::setPin(int pin, bool state)
{
    QSharedPointer<GPIOLine> line = exportedPins[pin];
    if (line)
        line->setPin(state);
}

bool PiGPIO::readPin(int pin)
{
    QSharedPointer<GPIOLine> line = exportedPins[pin];
    if (line)
        return line->readPin();    // read the buffered value

    return false;
}

