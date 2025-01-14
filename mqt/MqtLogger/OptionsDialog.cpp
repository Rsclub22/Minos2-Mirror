#include <QScrollArea>
#include <QStyleOption>

#include "regsettings.h"
#include "Clusterbandmapconfigure.h"
#include "defdirsdlg.h"
#include "DisplayOptions.h"
#include "radiosettingdialog.h"
#include "WsjtxConfigure.h"
#include "n1mmbroadcastconfig.h"
#include "BandsSelect.h"
#include "MinosLoggerEvents.h"
#include "MShowMessageDlg.h"
#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("OptionsDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MainRaised, this, &OptionsDialog::onMainRaised);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}
void OptionsDialog::onMainRaised()
{
    raise();
}
void OptionsDialog::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("OptionsDialog/geometry", saveGeometry());
}
void OptionsDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void OptionsDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void OptionsDialog::addTab(QFrame *f, const QString &t)
{
    QScrollArea *elementScrollArea = new QScrollArea();
    elementScrollArea->setWidgetResizable(true);
    elementScrollArea->setFocusPolicy(Qt::NoFocus);
    elementScrollArea->setFrameStyle(QStyleOptionFrame::None);
    elementScrollArea->setFrameShadow(QFrame::Plain);

    ui->optionTabs->addTab(elementScrollArea, t);

    elementScrollArea->setWidget(f);

}
int OptionsDialog::exec()
{
    RegSettings settings;
    int curTabNo = settings.getSettings().value("OptionsDialog/curTab").toInt();

    bs = new BandsSelect();
    cbc = new ClusterBandmapConfigure();
    ddd = new DefDirsDlg();
    dod = new DisplayOptions();
    rdc = new RadioSettingDialog ();
    nbc = new N1MMBroadcastConfig();
    wc  = new WsjtxConfigure();

    bs->initialise();
    bs->setAutoFillBackground(true);

    cbc->initialise();
    cbc->setAutoFillBackground(true);
    ddd->initialise();
    ddd->setAutoFillBackground(true);
    dod->initialise();
    dod->setAutoFillBackground(true);
    rdc->initialise();
    rdc->setAutoFillBackground(true);
    nbc->initialise();
    nbc->setAutoFillBackground(true);
    wc->initialise();
    wc->setAutoFillBackground(true);

    addTab(bs, tr("Wanted Bands"));
    addTab(cbc, tr("Cluster/Bandmap"));
    addTab(ddd, tr("General Options"));
    addTab(dod, tr("Display Options"));
    addTab(rdc, tr("Log Radio Settings"));
    addTab(nbc, tr("UDP Broadcast"));
    addTab(wc, tr("WSJT-X"));

    ui->optionTabs->setCurrentIndex(curTabNo);
    current = ui->optionTabs->currentWidget();

    int ret = QDialog::exec();

    if (ret == QDialog::Accepted)
    {
        bs->finalise();
        cbc->finalise();
        ddd->finalise();
        dod->finalise();
        rdc->finalise();
        nbc->finalise();
        wc->finalise();

    }
    return ret;

}
bool OptionsDialog::check()
{
    return cbc->check()
           && ddd->check()
           && dod->check()
           && rdc->check()
           && nbc->check()
           && wc->check()
           && bs->check() ;
}
void OptionsDialog::on_OKButton_clicked()
{
    if (check())
    {
        accept();
    }
}

void OptionsDialog::on_cancelButton_clicked()
{
    cbc->cancel();
    ddd->cancel();
    dod->cancel();
    rdc->cancel();
    nbc->cancel();
    wc->cancel();
    bs->cancel();
    reject();
}

void OptionsDialog::on_optionTabs_currentChanged(int index)
{
    if (current == bs && bs->checkChanged())
    {
        mShowMessage(tr("We will save the band changes before continuing."), this);
        accept();
    }

    RegSettings settings;
    settings.getSettings().setValue("OptionsDialog/curTab", index);

    current = ui->optionTabs->currentWidget();
}
