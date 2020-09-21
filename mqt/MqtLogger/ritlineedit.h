#ifndef RITLINEEDIT_H
#define RITLINEEDIT_H

#include <QLineEdit>
#include "rigcommon.h"



class RitLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    RitLineEdit(QWidget *parent);
    ~RitLineEdit();
    void changeFreq(bool direction);
    void setRitOnFlag(bool state);

    void setTensKhz(bool tensKhz);
    void setMaxRit(int maxRit_);
    void setMinRit(int minRit_);

signals:
    void receivedFocus() ;
    void lostFocus();
    void newFreq(ShortFreq);
    void freqEditReturn();
private:

    bool ritOn = false;
    bool ritTenKHz = false;
    int maxRit = MAX_RITFREQ;
    int minRit = MAX_RITFREQ * -1;
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);


} ;


#endif // RITLINEEDIT_H
