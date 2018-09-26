#ifndef QLOGTABWIDGET_H
#define QLOGTABWIDGET_H

#include "base_pch.h"

/**
 *  Derived Class from QTabWidget.
 *  Protected QTabWidget::tabBar() method is
 *  overridden in order to make it accessible.
 *  Also 'currentChanged(int)' signal is connected to custom Slot.
 */

class QLogTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit QLogTabWidget(QWidget *parent = nullptr);
    virtual ~QLogTabWidget() override
    {}

    //Overridden method from QTabWidget
    QTabBar* tabBar();

    void setTabColor(int index, QColor colour);



public slots:
};

#endif // QLOGTABWIDGET_H
