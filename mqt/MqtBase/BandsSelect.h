#ifndef BANDSSELECT_H
#define BANDSSELECT_H

#include "base_pch.h"

namespace Ui {
class BandsSelect;
}

class BandsSelect : public QDialog
{
    Q_OBJECT

public:
    explicit BandsSelect(QWidget *parent = nullptr);
    ~BandsSelect();

private slots:
    void on_cancelButton_clicked();

    void on_OKButton_clicked();

    void cbChanged(bool);
private:
    Ui::BandsSelect *ui;
    QVector<QSharedPointer<BandInfo> > bands;

    QMap<QCheckBox *, QSharedPointer<BandInfo>> cbbands;

    void initCheckBoxes();
};

#endif // BANDSSELECT_H
