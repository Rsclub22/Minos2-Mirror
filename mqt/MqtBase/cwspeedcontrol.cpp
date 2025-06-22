#include "CwSpeedControl.h"
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>

CwSpeedControl::CwSpeedControl(QWidget *parent)
    : QWidget(parent)
{
    slider = new QSlider(Qt::Horizontal);
    slider->setRange(5, 30);
    slider->setValue(20);

    speedLabel = new QLabel("CW Speed: 20 WPM");

    connect(slider, &QSlider::valueChanged, this, &CwSpeedControl::onSpeedChanged);

    auto *layout = new QVBoxLayout;
    layout->addWidget(speedLabel);
    layout->addWidget(slider);
    setLayout(layout);
}

void CwSpeedControl::setSpeedRange(int min, int max)
{
    slider->setRange(min, max);
}

void CwSpeedControl::setValue(int speed)
{
    slider->setValue(20);
}

int CwSpeedControl::getValue()
{
    return slider->value();
}


int CwSpeedControl::cwSpeed() const
{
    return slider->value();
}

void CwSpeedControl::onSpeedChanged(int value)
{
    speedLabel->setText(QString("CW Speed: %1 WPM").arg(value));
    emit cwSpeedChanged(value);
}


