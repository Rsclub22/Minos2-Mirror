/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include <QtWidgets>

class VolumeSlider : public QSlider
{
    Q_OBJECT

public:
    explicit VolumeSlider(QWidget *parent = nullptr);
    ~VolumeSlider() override;

    void setVolume(int level);


signals:
    void sendVolumeRadio(int);

private slots:

    void setRadioVol();
    void setRadioVol(int level);
private:
    bool valueFromRadio = false;
    bool volSliderSelected = false;
    int curVolLevel = -1;           // -1, volsider not visible



};

#endif // VOLUMESLIDER_H
