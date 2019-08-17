/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef MINOSCOMPASS_H
#define MINOSCOMPASS_H

//#include <QWidget>
#include <QDial>

class MinosCompass : public QDial
{
    Q_OBJECT

public:
    MinosCompass(QWidget *parent = nullptr);

public slots:

    void compassDialUpdate(int);
/*
// for test
    void upDateDial();

signals:
    void bearing_updated(const QString);

// for test
*/
signals:
    void sendClickBearing(int brg);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;


    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int compassDialBearing;
    int mouseBearing = -1;

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
};


#endif
