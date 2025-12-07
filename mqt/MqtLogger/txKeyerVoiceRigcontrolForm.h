#ifndef TXKEYERVOICERIGCONTROLFORM_H
#define TXKEYERVOICERIGCONTROLFORM_H

#include <QWidget>

namespace Ui {
class TxVoiceRigControlForm;
}

class TxVoiceRigControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit TxVoiceRigControlForm(QWidget *parent = nullptr);
    ~TxVoiceRigControlForm();

private:
    Ui::TxVoiceRigControlForm *ui;
};

#endif // TXKEYERVOICERIGCONTROLFORM_H
