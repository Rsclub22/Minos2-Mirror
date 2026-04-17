#include <QCheckBox>
#include "BandList.h"
#include "cutils.h"
#include "tlogcontainer.h"
#include "ContestApp.h"
#include "waitcursor.h"
#include "ConfigFile.h"
#include "BandsSelect.h"
#include "ui_BandsSelect.h"

BandsSelect::BandsSelect(QWidget *parent) :
    MinosPanel(parent),
    ui(new Ui::BandsSelect)
{
    ui->setupUi(this);
}

BandsSelect::~BandsSelect()
{
    delete ui;
}

void BandsSelect::initialise()
{
    initCheckBoxes();
}
bool BandsSelect::check()
{
    return true;
}
void BandsSelect::cancel()
{

}

bool BandsSelect::checkChanged()
{
   bool  changed = false;
    for (auto cb = cbbands.constBegin(); cb != cbbands.constEnd(); cb++)
    {
        if (cb.value()->enabled != cb.key()->isChecked())
        {
            return true;
        }
    }
    return changed;
}
void BandsSelect::finalise()
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

        TWaitCursor wc(this);
        MinosConfig::getMinosConfig() ->bounce();
        LogContainer->selectSession(TContestApp::getContestApp()->currSession);
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

    for (auto &b: bands)
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

