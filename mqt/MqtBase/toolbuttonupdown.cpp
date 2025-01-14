#include "toolbuttonupdown.h"
#include "rotatorcommon.h"

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
    valueDisplay = new QLabel();
    setBearingText(currentValue);
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
    valueDisplay->setText(convertBearingToString(num));
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
    /*
    increment(); // Immediate action on press
    incrementTimer->start();
    */
    pressTimer.start(); // Start tracking press duration
    increment(); // Immediate action on press
    QTimer::singleShot(200, this, [this]()
    {
        // Start timer only if the button is still pressed after 200 ms
        if (upButton->isDown())
        {
            incrementTimer->start();
        }
    });

}

void ToolButtonUpDown::startDecrement()
{
    /*
     * decrement(); // Immediate action on press
    decrementTimer->start();
    */
    pressTimer.start(); // Start tracking press duration
    decrement(); // Immediate action on press
    QTimer::singleShot(200, this, [this]()
    {
        // Start timer only if the button is still pressed after 200 ms
        if (downButton->isDown())
        {
            decrementTimer->start();
        }
    });
}

void ToolButtonUpDown::stopAction()
{
    incrementTimer->stop();
    decrementTimer->stop();
}
