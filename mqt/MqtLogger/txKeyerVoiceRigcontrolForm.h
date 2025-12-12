#ifndef TXKEYERVOICERIGCONTROLFORM_H
#define TXKEYERVOICERIGCONTROLFORM_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "keyerindicatorswidget.h"
#include "pttindicatorwidget.h"
#include "keyererrormessagewidget.h"

class TxVoiceRigControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit TxVoiceRigControlForm(QWidget *parent = nullptr);
    ~TxVoiceRigControlForm();

    void setKeyerAvailableIndicatorOnOff(bool on);
    void setRepeatIndicatorOnOff(bool on);
    void setEOMLabelText(const QString text);
    void clearEOMLabelText();
    void setPttEnabledIndicatorOnOff(const bool on);
    void setTxStatusIndicatorOnOff(const bool on);
    void setPttTypeText(const QString text);
    void clearPttTypeText();
    void setErrorMessageDisplayText(const QString errormsg);
    void clearErrorMessageDisplayText();
    void showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour);

private:


    QVBoxLayout *mainLayout;

    QHBoxLayout *indicatorLayout;

    KeyerIndicatorsWidget *keyerIndicatorsWidget;
    PttIndicatorWidget *pttIndicatorWidget;

    QHBoxLayout *errorMessageLayout;
    KeyerErrorMessageWidget *keyerErrorMessageWidget;

};

#endif // TXKEYERVOICERIGCONTROLFORM_H
