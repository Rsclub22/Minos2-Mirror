#ifndef TOOLBUTTONUPDOWN_H
#define TOOLBUTTONUPDOWN_H

#include <QWidget>
#include <QElapsedTimer>
#include "rotatorcommon.h"

class QToolButton;
class QLabel;
class QTimer;

class ToolButtonUpDown : public QFrame
{
    Q_OBJECT

public:
    explicit ToolButtonUpDown(QWidget *parent = nullptr);
    int value() const;
    void setValue(int newValue);
    void setRange(int min, int max);
    int getMin();
    int getMax();
    void setStep(int step);
    int getStep();
    void setText(const QString &text);
    void setBearingText(const int num);
    void setSouthStopType(enum southStop southStopType_);

    void setValueDisplay(bool visible);
signals:
    void valueChanged(int newValue);

private slots:
    void increment();
    void decrement();
    void startIncrement();
    void startDecrement();
    void stopAction();

private:
    QToolButton *upButton;
    QToolButton *downButton;
    QLabel *valueDisplay;
    QTimer *incrementTimer;
    QTimer *decrementTimer;
    QElapsedTimer pressTimer;


    int currentValue = 0;
    int minimum;
    int maximum;
    int step;
    enum southStop southStopType = southStop::S_STOPOFF;
};





#endif // TOOLBUTTONUPDOWN_H
