#ifndef TXKEYERNONEFORM_H
#define TXKEYERNONEFORM_H

#include <QWidget>

namespace Ui {
class TxKeyerNoneForm;
}
class TxKeyerNoneForm : public QWidget
{
    Q_OBJECT

public:
    explicit TxKeyerNoneForm(QWidget *parent = nullptr);
    ~TxKeyerNoneForm();

private:
    Ui::TxKeyerNoneForm *ui;
};

#endif // TXKEYERNONEFORM_H
