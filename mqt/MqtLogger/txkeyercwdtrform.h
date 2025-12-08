#ifndef TXKEYERCWDTRFORM_H
#define TXKEYERCWDTRFORM_H

#include <QWidget>
#include "cwspeedcontrol.h"

namespace Ui {
class TxKeyerCwDtrForm;
}

class TxKeyerCwDtrForm : public QWidget
{
    Q_OBJECT

public:
    explicit TxKeyerCwDtrForm(QWidget *parent = nullptr);
    ~TxKeyerCwDtrForm();

    void setCwFreeTextIndicatorOnOff(bool on);


signals:
    void sendWpmToPcCwkeyer(int wpm);

private:
    Ui::TxKeyerCwDtrForm *ui;

    CwSpeedControl *cwSpeedSlider = nullptr;

    void createCwSlider();

};

#endif // TXKEYERCWDTRFORM_H
