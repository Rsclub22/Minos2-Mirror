/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
/////////////////////////////////////////////////////////////////////////////


#include "volumeslider.h"
#include "rigcontrolcommonconstants.h"

VolumeSlider::VolumeSlider(QWidget *parent):
    QSlider(parent)
{
    setMaximum(SLIDERMAX);
    setMinimum(0);
    setFocusPolicy(Qt::StrongFocus);



    connect(this, SIGNAL(sliderReleased()), this, SLOT(setRadioVol()));
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(setRadioVol(int)));



}

VolumeSlider::~VolumeSlider()
{

}






// from radio

void VolumeSlider::setVolume(int level)
{
    valueFromRadio = true;
    setValue(level);
}




// to radio

void VolumeSlider::setRadioVol()
{
    if (!valueFromRadio)
    {
        int level = value();
        emit sendVolumeRadio(level);
    }

    valueFromRadio = false;


}

void VolumeSlider::setRadioVol(int level)
{
    if (!valueFromRadio)
    {
        emit sendVolumeRadio(level);
    }

    valueFromRadio = false;
}


