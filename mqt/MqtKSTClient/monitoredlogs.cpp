#include "monitoredlogs.h"
#include "ui_monitoredlogs.h"

MonitoredLogs::MonitoredLogs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonitoredLogs)
{
    ui->setupUi(this);

//    treeModel = new MonitorTreeModel();
//    ui->monitorTree->setModel(treeModel);
//    ui->monitorTree->header()->show();

}

MonitoredLogs::~MonitoredLogs()
{
    delete ui;
}
