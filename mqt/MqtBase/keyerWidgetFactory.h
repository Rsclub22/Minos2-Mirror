#ifndef KEYERWIDGETFACTORY_H
#define KEYERWIDGETFACTORY_H

#include <QWidget>
#include "keyerindicatorswidget.h"
#include "pttindicatorwidget.h"
#include "keyererrormessagewidget.h"
#include "cwspeedcontrol.h"
#include "cwentrywidget.h"

// Structs to group related widgets for convenience
struct KeyerIndicators {
    KeyerIndicatorsWidget* keyerIndicators;
    PttIndicatorWidget* pttIndicator;
};

struct CwMessagePlayingRow {
    QHBoxLayout* layout;
    QLabel* label;
    QLabel* display;
};



// Factory functions to create commonly used widgets
namespace KeyerWidgetFactory {

KeyerIndicators createIndicators(QWidget* parent = nullptr);


KeyerErrorMessageWidget* createErrorMessage(QWidget* parent = nullptr);

CwSpeedControl* createCwSpeedControl(QWidget* parent = nullptr);

CwEntryWidget* createCwEntry(QWidget* parent = nullptr);

CwMessagePlayingRow createCwMessagePlayingRow(QWidget* parent = nullptr);

QHBoxLayout* createRowLayout(QWidget* parent = nullptr, int left=2, int top=0, int right=0, int bottom=2, int spacing=6);

QVBoxLayout* createMainLayout(QWidget* parent = nullptr, int left = 0, int top = 0, int right = 0, int bottom = 0, int spacing= 2);

}


#endif // KEYERWIDGETFACTORY_H
