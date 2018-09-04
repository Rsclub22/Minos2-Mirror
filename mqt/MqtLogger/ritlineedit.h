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

signals:
    void receivedFocus() ;
    void lostFocus();
    void newFreq(QString);
    void freqEditReturn();
private:

    bool ritOn = false;
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);


} ;


#endif // RITLINEEDIT_H
