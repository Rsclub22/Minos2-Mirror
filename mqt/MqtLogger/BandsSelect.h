#ifndef BANDSSELECT_H
#define BANDSSELECT_H

#include "BandList.h"
#include <QFrame>
class QCheckBox;

namespace Ui {
class BandsSelect;
}

class BandsSelect : public QFrame
{
    Q_OBJECT

public:
    explicit BandsSelect(QWidget *parent = nullptr);
    ~BandsSelect();

    void initialise();
    void finalise();

    bool check();
    void cancel();

    bool checkChanged();
private slots:
    void on_allHFButton_clicked();

    void on_allVHFButton_clicked();

    void on_allMWButton_clicked();

private:
    Ui::BandsSelect *ui;
    QVector<QSharedPointer<BandInfo> > bands;

    QMap<QCheckBox *, QSharedPointer<BandInfo>> cbbands;

    void initCheckBoxes();
    bool areAnyBandsChecked(QString bandType);
    void changeAll(QString bandType);
};

#endif // BANDSSELECT_H
