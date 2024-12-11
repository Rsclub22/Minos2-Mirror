/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      BearingLineEdit
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef BEARINGLINEEDIT_H
#define BEARINGLINEEDIT_H

#include <QObject>
#include "rotatorcommon.h"
//#include "cutils.h"


#include <QLineEdit>
#include <QRegularExpression>
#include <QPalette>

class BearingLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit BearingLineEdit(QWidget *parent = nullptr);

    bool isValid() const;           // Check if the current value is valid
    int getBearing() const;         // Get the current bearing as an integer

    void setMaxMinBearing(int maxBrg, int minBrg);

protected:
    void showBearingGoodBad(bool state);  // Update visual appearance based on validity
    void changeEvent(QEvent *event) override;

private slots:
    void onTextChanged(const QString &text);  // Handle text changes for validation

private:
    bool bearingValid;              // Flag to indicate if the current value is valid
    int maxBearing = COMPASS_MAX360;
    int minBearing = COMPASS_MIN0;
};



#endif // BEARINGLINEEDIT_H
