#include "indicatorpushbutton.h"

IndicatorPushButton::IndicatorPushButton(QWidget* parent): QPushButton(parent)
{
    buttonSize = QSize(16, 16);
}


IndicatorPushButton::~IndicatorPushButton()
{

}


QSize IndicatorPushButton::sizeHint() const
{
    return buttonSize;
}


QSize IndicatorPushButton::minimumSizeHint() const
{

    return buttonSize;
}
