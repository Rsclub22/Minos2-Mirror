#include "toolbuttonupdown.h"

#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QTimer>

ToolButtonUpDown::ToolButtonUpDown(QWidget *parent)
    : QWidget(parent), currentValue(0), minimum(0), maximum(100), step(1)
{
    // Initialize buttons
    upButton = new QToolButton(this);
    upButton->setArrowType(Qt::UpArrow);

    downButton = new QToolButton(this);
    downButton->setArrowType(Qt::DownArrow);

    // Initialize display
    //valueDisplay = new QLineEdit(QString::number(currentValue), this);
    valueDisplay = new QLabel();
    setBearingText(currentValue);
    //valueDisplay->setValidator(new QIntValidator(minimum, maximum, this));
    valueDisplay->setAlignment(Qt::AlignCenter);
    valueDisplay->setFixedWidth(50);

    // Initialize timers
    incrementTimer = new QTimer(this);
    decrementTimer = new QTimer(this);
    incrementTimer->setInterval(150); // Adjust interval for desired speed
    decrementTimer->setInterval(150);

    // Layout setup
    QVBoxLayout *buttonsLayout = new QVBoxLayout;
    buttonsLayout->addWidget(upButton);
    buttonsLayout->addWidget(downButton);
    buttonsLayout->setSpacing(0);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(valueDisplay);
    mainLayout->addLayout(buttonsLayout);

    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);

    // Connect signals and slots
    connect(upButton, &QToolButton::pressed, this, &ToolButtonUpDown::startIncrement);
    connect(upButton, &QToolButton::released, this, &ToolButtonUpDown::stopAction);
    connect(downButton, &QToolButton::pressed, this, &ToolButtonUpDown::startDecrement);
    connect(downButton, &QToolButton::released, this, &ToolButtonUpDown::stopAction);
/*
    connect(valueDisplay, &QLineEdit::editingFinished, [this]() {
        bool ok;
        int val = valueDisplay->text().toInt(&ok);
        if (ok) {
            if (val < minimum) val = minimum;
            if (val > maximum) val = maximum;
            setValue(val);
        } else {
            setValue(currentValue); // Reset to previous value if invalid input
        }
    });
*/
    connect(incrementTimer, &QTimer::timeout, this, &ToolButtonUpDown::increment);
    connect(decrementTimer, &QTimer::timeout, this, &ToolButtonUpDown::decrement);
}

int ToolButtonUpDown::value() const
{
    return currentValue;
}

void ToolButtonUpDown::setText(const QString &text)
{
    valueDisplay->setText(text);

}

void ToolButtonUpDown::setBearingText(const int num)
{
    QString text = QString::number(num);
    if (num < 10)
    {
        text = text.prepend("00");
    }
    else if (num < 100)
    {
        text = text.prepend("0");
    }

    valueDisplay->setText(text);

}

void ToolButtonUpDown::setValue(int newValue)
{
    if (newValue < minimum)
        newValue = minimum;
    if (newValue > maximum)
        newValue = maximum;

    if (currentValue != newValue)
    {
        currentValue = newValue;
        setBearingText(currentValue);
        emit valueChanged(currentValue);
    }
}

void ToolButtonUpDown::setRange(int min, int max)
{
    minimum = min;
    maximum = max;
    //valueDisplay->setValidator(new QIntValidator(minimum, maximum, this));
    setValue(currentValue); // Ensure currentValue is within new range
}

void ToolButtonUpDown::setStep(int stepValue)
{
    step = stepValue;
}

void ToolButtonUpDown::increment()
{
    setValue(currentValue + step);
}

void ToolButtonUpDown::decrement()
{
    setValue(currentValue - step);
}

void ToolButtonUpDown::startIncrement()
{
    increment(); // Immediate action on press
    incrementTimer->start();
}

void ToolButtonUpDown::startDecrement()
{
    decrement(); // Immediate action on press
    decrementTimer->start();
}

void ToolButtonUpDown::stopAction()
{
    incrementTimer->stop();
    decrementTimer->stop();
}
