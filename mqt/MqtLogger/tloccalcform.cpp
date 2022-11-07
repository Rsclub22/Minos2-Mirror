#include <QSettings>

#include "tloccalcform.h"
#include "ui_tloccalcform.h"

TLocCalcForm::TLocCalcForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TLocCalcForm)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QSettings settings;
    QByteArray geometry = settings.value("TLocCalcForm/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

}

TLocCalcForm::~TLocCalcForm()
{
    delete ui;
}
int TLocCalcForm::exec()
{
    ui->locFrame->S1Loc = S1Loc;

    ui->locFrame->doExec();

    int ret = QDialog::exec();

    Distance = ui->locFrame->Distance;
    return ret;
}

void TLocCalcForm::doCloseEvent()
{
    QSettings settings;
    settings.setValue("TLocCalcForm/geometry", saveGeometry());
}
void TLocCalcForm::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TLocCalcForm::accept()
{
    doCloseEvent();
    QDialog::accept();
}
