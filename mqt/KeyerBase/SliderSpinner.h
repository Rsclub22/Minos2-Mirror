#ifndef SLIDERSPINNER_H
#define SLIDERSPINNER_H

#include <QFrame>
#include <QSpinBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace Ui {
class SliderSpinner;
}

class SliderSpinner : public QFrame
{
    Q_OBJECT

public:
    explicit SliderSpinner(QWidget *parent, QString label, Qt::Orientation vh, double minVal, double maxVal, double init);
    virtual ~SliderSpinner() override;

    void setValue(double val);
    void setIntValue(int val);
    double getValue();
    int getIntValue();

    double maximum()
    {
        return maxVal;
    }
    double minimum()
    {
        return minVal;
    }

private:
    Ui::SliderSpinner *ui;

    QSlider *qsl;
    QDoubleSpinBox *qsp;

    QBoxLayout *bl;

    double maxVal;
    double minVal;

signals:
    void valueChanged();

private slots:
    void setSliderValue(double v);
    void setSpinnerValue(int v);
};

#endif // SLIDERSPINNER_H
