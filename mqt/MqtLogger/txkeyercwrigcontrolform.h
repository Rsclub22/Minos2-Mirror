#ifndef TXKEYERCWRIGCONTROLFORM_H
#define TXKEYERCWRIGCONTROLFORM_H

#include <QWidget>
#include "cwentrywidget.h"
#include "keyererrormessagewidget.h"
#include "keyerWidgetFactory.h"

class TxKeyerCwRigControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit TxKeyerCwRigControlForm(QWidget *parent = nullptr);
    ~TxKeyerCwRigControlForm();

    void setCwFreeTextIndicatorOnOff(bool on);
    void setCwSliderValue(int value);

    QString getCwEntryText();

    void setKeyerAvailableIndicatorOnOff(bool on);
    void setRepeatIndicatorOnOff(bool on);
    void setEOMLabelText(const QString text);
    void clearEOMLabelText();


    void setPttEnabledIndicatorOnOff(const bool on);
    void setTxStatusIndicatorOnOff(const bool on);
    void setPttTypeText(const QString text);
    void clearPttTypeText();
    void setStoredMessagePlayingDisplay(const QString msg);

    void clearStoredMessagePlayingDisplay();

    void setErrorMessageDisplayText(const QString errormsg);
    void clearErrorMessageDisplayText();


    void showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour);
    void selectAllText();
signals:


    void cwEntryReturnPressed();

private:

    QHBoxLayout *indicatorLayout;
    KeyerIndicators indicators;


    //QHBoxLayout *keyerErrorMessageLayout;
    //KeyerErrorMessageWidget *keyerErrorMessageDisplay;


    CwMessagePlayingRow cwMessagePlayingRow;


    QHBoxLayout *cwEntryLayout;
    CwEntryWidget *cwEntry;


};

#endif // TXKEYERCWRIGCONTROLFORM_H
