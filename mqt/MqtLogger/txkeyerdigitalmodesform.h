#ifndef TXKEYERDIGITALMODESFORM_H
#define TXKEYERDIGITALMODESFORM_H

#include <QWidget>

namespace Ui {
class TxKeyerDigitalModesForm;
}

class TxKeyerDigitalModesForm : public QWidget
{
    Q_OBJECT

public:
    explicit TxKeyerDigitalModesForm(QWidget *parent = nullptr);
    ~TxKeyerDigitalModesForm();

private:
    Ui::TxKeyerDigitalModesForm *ui;
};

#endif // TXKEYERDIGITALMODESFORM_H
