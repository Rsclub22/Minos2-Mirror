#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>

class VolumeSlider : public QSlider
{
    Q_OBJECT

public:
    explicit VolumeSlider(QWidget *parent = nullptr);
    ~VolumeSlider();

    void setVolume(int level);


signals:
    void sendVolumeRadio(int);

private slots:
    void volSliderPressed();
    void volSliderReleased();
    void setRadioVol();
    void setRadioVol(int level);
private:

    bool volSliderSelected = false;
    int curVolLevel = -1;           // -1, volsider not visible




};

#endif // VOLUMESLIDER_H
