#ifndef TXKEYERCWDTRFORM_H
#define TXKEYERCWDTRFORM_H

#include <QWidget>

namespace Ui {
class TxKeyerCwDtrForm;
}

class TxKeyerCwDtrForm : public QWidget
{
    Q_OBJECT

public:
    explicit TxKeyerCwDtrForm(QWidget *parent = nullptr);
    ~TxKeyerCwDtrForm();

private:
    Ui::TxKeyerCwDtrForm *ui;
};

#endif // TXKEYERCWDTRFORM_H
