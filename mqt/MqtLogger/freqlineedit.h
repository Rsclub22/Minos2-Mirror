/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef FREQLINEEDIT_H
#define FREQLINEEDIT_H

#include <QLineEdit>

class FreqLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    FreqLineEdit(QWidget *parent);
    ~FreqLineEdit();
    void changeFreq(bool direction);
    //QString convertFreqString(double frequency);

signals:

    void newFreq();
    void freqEditReturn();
private:


    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;



} ;

#endif // FREQLINEEDIT_H
