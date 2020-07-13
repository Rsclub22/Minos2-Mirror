#ifndef RITLINEEDIT_H
#define RITLINEEDIT_H

#include <QLineEdit>



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
    void newFreq(int);
    void freqEditReturn();
private:

    bool ritOn = false;
    bool ritTenKHz = false;
    int maxRit = 9999;
    int minRit = -9999;
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);


} ;


#endif // RITLINEEDIT_H
