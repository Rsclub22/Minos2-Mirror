#ifndef TXKEYERCWRIGCONTROLFORM_H
#define TXKEYERCWRIGCONTROLFORM_H

#include <QWidget>

namespace Ui {
class TxKeyerCwRigControlForm;
}

class TxKeyerCwRigControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit TxKeyerCwRigControlForm(QWidget *parent = nullptr);
    ~TxKeyerCwRigControlForm();

private:
    Ui::TxKeyerCwRigControlForm *ui;
};

#endif // TXKEYERCWRIGCONTROLFORM_H
