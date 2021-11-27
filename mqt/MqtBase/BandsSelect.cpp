#include "base_pch.h"
#include <QCheckBox>
#include "bandmapcommon.h"
#include "BandList.h"
#include "cutils.h"

#include "BandsSelect.h"
#include "ui_BandsSelect.h"

BandsSelect::BandsSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BandsSelect)
{
    ui->setupUi(this);
    initCheckBoxes();
}

BandsSelect::~BandsSelect()
{
    delete ui;
}

void BandsSelect::on_cancelButton_clicked()
{
    reject();
}


void BandsSelect::on_OKButton_clicked()
{
    bool changed = false;
    for (auto cb = cbbands.constBegin(); cb != cbbands.constEnd(); cb++)
    {
        if (cb.value()->enabled != cb.key()->isChecked())
        {
            cb.value()->enabled = cb.key()->isChecked();
            changed = true;
        }
    }
    if (changed)
    {
        BandList::getBandList().updateEnabled();
        accept();
    }
    else
    {
        reject();
    }
}

void BandsSelect::initCheckBoxes()
{
    QGridLayout *hfBandsLayout = new QGridLayout();
    ui->hfBandsFrame->setLayout(hfBandsLayout);
    QGridLayout *vhfBandsLayout = new QGridLayout();
    ui->vhfBandsFrame->setLayout(vhfBandsLayout);
    QGridLayout *mwBandsLayout = new QGridLayout();
    ui->mwBandsFrame->setLayout(mwBandsLayout);

    int hfRow = 0;
    int hfCol = 0;
    int vhfRow = 0;
    int vhfCol = 0;
    int mwRow = 0;
    int mwCol = 0;

    BandList::getBandList().loadAllBands(bands, false); // don't filter on enabled

    for (auto const &b: qAsConst(bands))
    {
        QCheckBox *cb = new QCheckBox();

        cb->setText(b->uk);
        if (b->getType() == HF_BANDTYPE)
        {
            hfBandsLayout->addWidget(cb, hfRow, hfCol);
            hfRow++;
        }
        else if (b->getType() == VHF_BANDTYPE)
        {
            vhfBandsLayout->addWidget(cb, vhfRow, vhfCol);
            vhfRow++;
        }
        else if (b->getType() == MW_BANDTYPE)
        {
            mwBandsLayout->addWidget(cb, mwRow, mwCol);
            mwCol++;
            if (mwCol % 3 == 0)
            {
                mwCol = 0;
                mwRow++;
            }
        }

        cb->setChecked(b->enabled);

        cbbands[cb] = b;
    }
}

bool BandsSelect::areAnyBandsChecked(QString bandType)
{
    for (auto cb = cbbands.constBegin(); cb != cbbands.constEnd(); cb++)
    {
        if (cb.value()->getType() == bandType && cb.key()->isChecked())
        {
            return true;
        }
    }
    return false;
}
void BandsSelect::changeAll(QString( bandType))
{
    bool check = true;
    if (areAnyBandsChecked(bandType))
    {
        check = false;;
    }

    for (auto cb = cbbands.constBegin(); cb != cbbands.constEnd(); cb++)
    {
        if (cb.value()->getType() == bandType)
        {
            cb.key()->setChecked(check);
        }
    }
}
void BandsSelect::on_allHFButton_clicked()
{
    changeAll(HF_BANDTYPE);
}


void BandsSelect::on_allVHFButton_clicked()
{
    changeAll(VHF_BANDTYPE);
}


void BandsSelect::on_allMWButton_clicked()
{
    changeAll(MW_BANDTYPE);
}

