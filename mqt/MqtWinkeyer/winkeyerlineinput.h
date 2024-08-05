/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WINKEYERLINEINPUT_H
#define WINKEYERLINEINPUT_H

#include <QObject>
#include <QLineEdit>

class WinkeyerLineInput : public QLineEdit
{
    Q_OBJECT
public:
    explicit WinkeyerLineInput(QWidget *parent = nullptr);


    void setValidatorRange(int min, int max);
    int getMinValue();
    int getMaxValue();
protected:
    void keyPressEvent(QKeyEvent *event) override;


signals:

private:

    int minValue = 0;
    int maxValue = 250;
    bool validateInput();
};

#endif // WINKEYERLINEINPUT_H
