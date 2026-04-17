#include "kstmainwindow.h"
#include "kstbuttonsframe.h"
#include "ui_kstbuttonsframe.h"

KSTButtonsFrame::KSTButtonsFrame(QWidget *parent)
    : MinosPanel(parent)
    , ui(new Ui::KSTButtonsFrame)
{
    ui->setupUi(this);
}

KSTButtonsFrame::~KSTButtonsFrame()
{
    delete ui;
}
void KSTButtonsFrame::on_closeButton_clicked()
{
    mainWindow->do_closeButton_clicked();
}

void KSTButtonsFrame::on_FontChanged()
{

}
void KSTButtonsFrame::checkAwayButton()
{
    QSharedPointer<KstUser> user = mainWindow->getUser(KstUser(mainWindow->myCallsign, mainWindow->getActiveChat()));
    if (user)
    {
        if (user->away)
        {
            ui->awayButton->setText(tr("Set Back"));
        }
        else
        {
            ui->awayButton->setText(tr("Set Away"));
        }
    }
}

void KSTButtonsFrame::setConnected(bool connected)
{
    ui->connectButton->setText(connected?tr("Disconnect"):tr("Connect"));

}
void KSTButtonsFrame::on_connectButton_clicked()
{
    mainWindow->do_connectButton_clicked();
}


void KSTButtonsFrame::on_configureButton_clicked()
{
    mainWindow->do_configureButton_clicked();
}


void KSTButtonsFrame::on_layoutButton_clicked()
{
    mainWindow->do_layoutButton_clicked();
}


void KSTButtonsFrame::on_awayButton_clicked()
{
    mainWindow->do_awayButton_clicked();
}


void KSTButtonsFrame::on_logsButton_clicked()
{
    mainWindow->do_logsButton_clicked();
}


void KSTButtonsFrame::on_clearButton_clicked()
{
    mainWindow->do_clearLogsButton_clicked();
}

