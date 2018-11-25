#ifndef TMINOSHELPFORM_H
#define TMINOSHELPFORM_H

#include "base_pch.h"

namespace Ui {
class TMinosBSHelpForm;
}

class TMinosBSHelpForm : public QDialog
{
    Q_OBJECT

public:
    explicit TMinosBSHelpForm(QWidget *parent = nullptr);
    ~TMinosBSHelpForm();
    void setText(const QString &text);

private slots:
    void on_CloseButton_clicked();

private:
    Ui::TMinosBSHelpForm *ui;
};

#endif // TMINOSHELPFORM_H
