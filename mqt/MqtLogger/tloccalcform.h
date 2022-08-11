#ifndef TLOCCALCFORM_H
#define TLOCCALCFORM_H

#include <QDialog>

namespace Ui {
class TLocCalcForm;
}

class TLocCalcForm : public QDialog
{
    Q_OBJECT

public:
    explicit TLocCalcForm(QWidget *parent = nullptr);
    ~TLocCalcForm() override;
    virtual int exec() override;

    QString S1Loc;
    QString Distance;

private slots:

private:
    Ui::TLocCalcForm *ui;
    void doCloseEvent();

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;
};

#endif // TLOCCALCFORM_H
