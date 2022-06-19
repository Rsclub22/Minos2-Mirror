#include "mqtUtils_pch.h"

#include <QLabel>
#include "cutils.h"
#include "SliderSpinner.h"
#include "ui_SliderSpinner.h"

SliderSpinner::SliderSpinner(QWidget *parent, QString label, Qt::Orientation vh, double minVal, double maxVal, double init) :
    QFrame(parent),
    ui(new Ui::SliderSpinner),
    maxVal(maxVal),
    minVal(minVal)
{
    ui->setupUi(this);

    setContentsMargins(0, 0, 0, 0);

    qsl = new QSlider(this);
    qsl->setOrientation(vh);

    qsl->setMinimum(minVal * 10);
    qsl->setMaximum(maxVal * 10);
    qsl->setValue(init * 10);

    // need to add marker ticks
    int tickInterval = (maxVal - minVal)/2;
    qsl->setTickInterval(tickInterval);


    qsp = new QDoubleSpinBox(this);
    qsp->setAlignment(Qt::AlignCenter);
    qsp->setDecimals(1);
    qsp->setSingleStep(0.1);

    qsp->setMinimum(minVal);
    qsp->setMaximum(maxVal);
    qsp->setValue(init);

    QLabel *ql = new QLabel(this);
    ql->setText(label);
    ql->setAlignment( Qt::AlignCenter );

    if (vh == Qt::Horizontal)
    {
        qsl->setTickPosition(QSlider::TicksAbove);
        bl = new QHBoxLayout(this);
        bl->addWidget(ql);
        bl->addWidget(qsl);
        bl->addWidget(qsp);
    }
    else
    {
        qsl->setTickPosition(QSlider::TicksRight);
        bl = new QVBoxLayout(this);
        bl->addWidget(qsp);

        QHBoxLayout *hb = new QHBoxLayout();

        QSpacerItem *hs = new QSpacerItem(1, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        hb->addItem(hs);
        hb->addWidget(qsl);
        hs = new QSpacerItem(1, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        hb->addItem(hs);

        bl->addItem(hb);
        bl->addWidget(ql);
    }
    connect(qsp, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SliderSpinner::setSliderValue);
    connect(qsl, &QSlider::valueChanged, this, &SliderSpinner::setSpinnerValue);
}

SliderSpinner::~SliderSpinner()
{
    delete ui;
}

void SliderSpinner::setValue(double val)
{
    trace(QString("setValue %1").arg(val));
    qsp->setValue(val);
}

void SliderSpinner::setIntValue(int val)
{
    trace(QString("setIntValue %1 %2").arg(val).arg(val/10.0));
    qsp->setValue(val/10.0);
}

double SliderSpinner::getValue()
{
//    trace(QString("getValue %1").arg(qsp->value()));
    return qsp->value();
}
int SliderSpinner::getIntValue()
{
    double qspval = qsp->value();
    double qspval10 = qspval * 10;
    int qspint = int(qspval10);
//    trace(QString("getIntValue %1 %2 %3").arg(qspval).arg(qspval10).arg(qspint));
    return qspint;
}
void SliderSpinner::setSliderValue(double v)
{
    int slval = qsl->value();
    double spval = qsp->value();
    trace(QString("setSlider %1 %2 %3").arg(v).arg(slval).arg(spval));
    if (!almost_equal(v, slval * 10.0, 2))
    {
        double round = ((v > 0)?0.01:-0.01);
        int rounded = (v + round) * 10;
        trace(QString("setSlider %1 %2 %3 %4").arg(v).arg(slval).arg( rounded ).arg(spval));
        qsl->setValue((v + round) * 10);
        emit valueChanged();
    }
}
void SliderSpinner::setSpinnerValue(int v)
{
    double spval = qsp->value();
    trace(QString("setSpinner %1 %2").arg(v).arg(spval));
    if (!almost_equal(spval, v/10.0, 2))
    {
        trace(QString("setSpinner %1 %2 %3").arg(v).arg(spval).arg(int(v/10.0 )));
        qsp->setValue(v/10.0);
        emit valueChanged();
    }
}
