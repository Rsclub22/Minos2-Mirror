#include "grittyframe.h"
#include "qlineedit.h"
#include "ui_grittyframe.h"

GrittyFrame::GrittyFrame(QWidget *parent,  QTextEdit *rxChars, QLineEdit *sendEdit, QString fname) :
    QFrame(parent),
    ui(new Ui::GrittyFrame)
{
    ui->setupUi(this);
}

GrittyFrame::~GrittyFrame()
{
    delete ui;
}
