#ifndef TOOLBUTTONUPDOWN_H
#define TOOLBUTTONUPDOWN_H

#include <QWidget>
#include <QElapsedTimer>
#include "rotatorcommon.h"

class QToolButton;
class QLabel;
class QTimer;

class ToolButtonUpDown : public QWidget
{
    Q_OBJECT

public:
    explicit ToolButtonUpDown(QWidget *parent = nullptr);
    int value() const;
    void setValue(int newValue);
    void setRange(int min, int max);
    void setStep(int step);
    void setText(const QString &text);
    void setBearingText(const int num);
    void setSouthStopType(enum southStop southStopType_);

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
