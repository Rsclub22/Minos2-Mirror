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

    // signals to prevent slider update from radio
    connect(this, SIGNAL(sliderPressed()), this, SLOT(volSliderPressed()));
    connect(this, SIGNAL(sliderReleased()), this, SLOT(volSliderReleased()));

}

VolumeSlider::~VolumeSlider()
{

}



void VolumeSlider::volSliderPressed()
{
    volSliderSelected = true;
}


void VolumeSlider::volSliderReleased()
{
    volSliderSelected = false;
}




// from radio

void VolumeSlider::setVolume(int level)
{


   if (!volSliderSelected)   // don't update level from radio if slider has focus
   {

       setValue(level);
   }


}

// to radio

void VolumeSlider::setRadioVol()
{
    int level = value();
    emit sendVolumeRadio(level);

}

void VolumeSlider::setRadioVol(int level)
{
    emit sendVolumeRadio(level);
}
