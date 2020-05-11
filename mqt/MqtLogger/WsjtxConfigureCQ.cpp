#include "ContestApp.h"
#include "cutils.h"

#include "WsjtxConfigureCQ.h"
#include "ui_WsjtxConfigureCQ.h"

WsjtxConfigureCQ::WsjtxConfigureCQ(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WsjtxConfigureCQ)
{
    ui->setupUi(this);

    QString testCQ;
    QString nontestCQ;

    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX1TestCQ, testCQ );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpWSJTX1NonTestCQ, nontestCQ );


    QStringList sl;
    CSVToStringList(testCQ, sl);

    for(auto s:sl)
    {
        ui->testCQ->appendPlainText(s);
    }

    CSVToStringList(nontestCQ, sl);

    for(auto s:sl)
    {
        ui->notTestCQ->appendPlainText(s);
    }

}

WsjtxConfigureCQ::~WsjtxConfigureCQ()
{
    delete ui;
}

void WsjtxConfigureCQ::on_OKButton_clicked()
{
    QStringList sl;
    QString s = ui->testCQ->toPlainText();
    s.remove("\r");
    sl = s.split("\n", QString::SkipEmptyParts);
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpWSJTX1TestCQ, sl.join(",") );

    s = ui->notTestCQ->toPlainText();
    s.remove("\r");
    sl = s.split("\n", QString::SkipEmptyParts);
    TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpWSJTX1NonTestCQ, sl.join(",") );

    accept();
}

void WsjtxConfigureCQ::on_cancelButton_clicked()
{
    reject();
}
