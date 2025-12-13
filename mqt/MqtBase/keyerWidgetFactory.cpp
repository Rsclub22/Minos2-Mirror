#include "KeyerWidgetFactory.h"

namespace KeyerWidgetFactory {

// Create the Keyer and Ptt indicators for a keyer
KeyerIndicators createIndicators(QWidget* parent)
{
    KeyerIndicators widgets;

    widgets.keyerIndicators = new KeyerIndicatorsWidget(parent);
    widgets.keyerIndicators->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    // optional: keep max height to sizeHint
    widgets.keyerIndicators->setMaximumHeight(widgets.keyerIndicators->sizeHint().height());

    widgets.pttIndicator = new PttIndicatorWidget(parent);
    widgets.pttIndicator->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    widgets.pttIndicator->setMaximumHeight(widgets.pttIndicator->sizeHint().height());

    return widgets;
}

// Create the error message widget
KeyerErrorMessageWidget* createErrorMessage(QWidget* parent)
{
    auto w = new KeyerErrorMessageWidget(parent);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    w->setMaximumHeight(w->sizeHint().height());
    return w;
}

// Create the CW speed control slider
CwSpeedControl* createCwSpeedControl(QWidget* parent)
{
    auto w = new CwSpeedControl(parent);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    w->setMaximumHeight(w->sizeHint().height());
    return w;
}

// Create the CW entry widget
CwEntryWidget* createCwEntry(QWidget* parent)
{
    auto w = new CwEntryWidget(parent);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    w->setMaximumHeight(w->sizeHint().height());
    return w;
}

QHBoxLayout* createRowLayout(QWidget* parent, int left, int top, int right, int bottom, int spacing)
{
    auto layout = new QHBoxLayout(parent);
    layout->setContentsMargins(left, top, right, bottom);
    layout->setSpacing(spacing);
    return layout;
}

QVBoxLayout* createMainLayout(QWidget* parent, int left, int top, int right, int bottom, int spacing)
{
    auto layout = new QVBoxLayout(parent);
    layout->setContentsMargins(left, top, right, bottom);
    layout->setSpacing(spacing);
    return layout;
}





CwMessagePlayingRow createCwMessagePlayingRow(QWidget* parent)
{
    CwMessagePlayingRow row;

    row.label = new QLabel(QObject::tr("Stored Message Playing:"), parent);
    row.label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    row.label->setMaximumHeight(row.label->sizeHint().height());

    row.display = new QLabel(parent);
    row.display->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    row.display->setMaximumHeight(row.display->sizeHint().height());

    row.layout = new QHBoxLayout();
    row.layout->setContentsMargins(2, 0, 0, 2);
    row.layout->setSpacing(6);
    row.layout->addWidget(row.label);
    row.layout->addWidget(row.display);

    return row;
}


} // namespace KeyerWidgetFactory
