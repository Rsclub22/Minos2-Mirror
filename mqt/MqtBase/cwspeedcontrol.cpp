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

    auto *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(speedLabel);
    layout->addWidget(slider);
    layout->addStretch();
    setLayout(layout);
}

void CwSpeedControl::setSpeedRange(int min, int max)
{
    slider->setRange(min, max);
}

void CwSpeedControl::setValue(int speed)
{
    if (slider->value() == speed)
    {
        return;
    }

    slider->blockSignals(true);
    slider->setValue(speed);
    speedLabel->setText(QString("CW Speed: %1 WPM").arg(speed));
    slider->blockSignals(false);

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


