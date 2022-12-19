#include "testframe.h"
#include "MShowMessageDlg.h"
#include "rxbuffer.h"
#include "MTrace.h"
#include "ui_testframe.h"

QStringList testData = {
"CQ CQ BARTG G0G0GJV G0GVV TEST",
"G0GJV DE PE1EWR PE1EWR",
"PE1EWR 599 001 001 001 K",
"599 020 020 020 PE1EWR",
"PE1EWR TU G0GJV QRX BARTG"

};

TestFrame::TestFrame(QWidget *parent, QLineEdit *sendEdit, QString fname) :
    QFrame(parent),
    ui(new Ui::TestFrame)
{
    ui->setupUi(this);

    for(const auto &s:qAsConst(testData))
    {
        trace(QString("Response %1").arg(s));
        bool newLine = true;
        for (auto c:qAsConst(s))
        {
            RXChar rxch(c, newLine, 0);
            newLine = false;
            RxBuffer::getRxBuffer()->addChar(rxch);
        }
    }
}

TestFrame::~TestFrame()
{
    delete ui;
}

void TestFrame::sendCharacters(const QString &toSend)
{
    mShowMessage(toSend, this);
}

void TestFrame::closeFrame()
{

}
