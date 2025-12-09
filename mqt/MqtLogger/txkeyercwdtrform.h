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
    void setCwSliderValue(int value);

    QString getCwEntryText();

    void setKeyerAvailableIndicatorOnOff(bool on);
    void setRepeatIndicatorOnOff(bool on);
    void setEOMLabelText(const QString text);
    void clearEOMLabelText();


    void setPttEnabledIndicator(const bool on);
    void setTxStatusIndicator(const bool on);
    void setPttTypeText(const QString text);
    void clearPttTypeText();
    void setStoredMessagePlayingDisplay(const QString msg);

    void clearStoredMessagePlayingDisplay(const QString msg);

    void setErrorMessageDisplayText(const QString errormsg);
    void clearErrorMessageDisplayText();


signals:

    void sendWpmToPcCwkeyer(int wpm);
    void cwEntryReturnPressed();

private:
    Ui::TxKeyerCwDtrForm *ui;

    CwSpeedControl *cwSpeedSlider = nullptr;

    void createCwSlider();

};

#endif // TXKEYERCWDTRFORM_H
