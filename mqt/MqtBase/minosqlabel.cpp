#include "minosqlabel.h"

MinosQLabel::MinosQLabel(QWidget* parent): QLabel(parent)
{

}

MinosQLabel::~MinosQLabel()
{

}

void MinosQLabel::mouseDoubleClickEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)
    emit mouseDoubleClicked();
}
