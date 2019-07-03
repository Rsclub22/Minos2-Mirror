#include "minoscontestloaddialog.h"
#include "ui_minoscontestloaddialog.h"

MinosContestLoadDialog::MinosContestLoadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MinosContestLoadDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
}

MinosContestLoadDialog::~MinosContestLoadDialog()
{
    delete ui;
}

void MinosContestLoadDialog::setLoadMessage(QString mess, bool newFile, bool list)
{
    QString m = "<h2><center>";

    m += newFile?"Creating ":"Loading ";

    m += list?"List file ":"Contest file ";
    m += "<p>";
    m += mess;

    ui->contestNameLabel->setText(m);
}
void MinosContestLoadDialog::doShow()
{
    show();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}
