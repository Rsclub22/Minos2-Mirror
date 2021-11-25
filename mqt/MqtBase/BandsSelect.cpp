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
    BandList::getBandList().updateEnabled();
    accept();
}

void BandsSelect::initCheckBoxes()
{
    QGridLayout *bandsLayout = new QGridLayout();
    ui->bandsFrame->setLayout(bandsLayout);
    int row = 0;
    int col = 0;

    BandList::getBandList().loadAllBands(bands, false); // don't filter on enabled

    for (auto const &b: qAsConst(bands))
    {
        QCheckBox *cb = new QCheckBox();

        cb->setText(b->uk);
        bandsLayout->addWidget(cb, row, col);
        col++;
        if (col %3 == 0)
        {
            col = 0;
            row++;
        }

        cb->setChecked(b->enabled);
        connect(cb, &QCheckBox::clicked, this, &BandsSelect::cbChanged);

        cbbands[cb] = b;
    }
}

void BandsSelect::cbChanged(bool)
{
    QCheckBox *s = dynamic_cast<QCheckBox *>(sender());
    // find the band fron the cb

    QSharedPointer<BandInfo> b = cbbands[s];
    b->enabled = s->isChecked();
}
