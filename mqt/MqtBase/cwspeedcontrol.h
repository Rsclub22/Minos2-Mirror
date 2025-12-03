#ifndef CWSPEEDCONTROL_H
#define CWSPEEDCONTROL_H

#include <QWidget>

class QSlider;
class QLabel;

class CwSpeedControl : public QWidget
{
    Q_OBJECT

public:
    explicit CwSpeedControl(QWidget *parent = nullptr);
    int cwSpeed() const;

    void setSpeedRange(int min, int max);
    void setValue(int speed);
    int getValue();

signals:
    void cwSpeedChanged(int wpm);

private slots:
    void onSpeedChanged(int value);

private:
    QSlider *slider;
    QLabel *speedLabel;
};

#endif // CWSPEEDCONTROL_H
