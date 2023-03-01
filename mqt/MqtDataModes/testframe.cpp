#include "MShowMessageDlg.h"
#include "rxbuffer.h"
#include "MTrace.h"
#include "dmmainwindow.h"
#include "enginewindow.h"
#include "testframe.h"
#include "ui_testframe.h"

QStringList testData = {
"CQ CQ BARTG G0G0GJV G0GVV TEST",
"G0GJV DE PE1EWR PE1EWR",
"PE1EWR 599 001 001 001 K",
"599 020 020 020 PE1EWR",
"PE1EWR TU G0GJV QRX BARTG"

};

// to run multiple copies - just do it!

TestFrame::TestFrame(EngineWindow *parent, QLineEdit */*sendEdit*/, QString /*fname*/, QString /*name*/) :
    QFrame(parent),
    ui(new Ui::TestFrame),
    engineWindow(parent)
{
    ui->setupUi(this);
    connect(mainWindow, &DMMainWindow::setSpeeds, this, &TestFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::sendCharactersDown, this, &TestFrame::onSendCharacters);
    connect(engineWindow, &EngineWindow::rigModeFreq, this, &TestFrame::onRigModeFreq);

    for(const auto &s:qAsConst(testData))
    {
        trace(QString("Response %1").arg(s));
        bool newLine = true;
        for (auto c:qAsConst(s))
        {
            RXChar rxch(c, newLine, 0, carrier);
            newLine = false;
            engineWindow->rxBuff.addChar(rxch);
        }
    }

    testTimer = new QTimer(this);
    connect(testTimer, &QTimer::timeout, this, &TestFrame::onTimeout);
//    testTimer->start(1000);
}

TestFrame::~TestFrame()
{
    delete ui;
}
void TestFrame::onTimeout()
{
    static int n = 0;
    QString s = QString::number(n);
    n++;
    bool newLine = true;
    for (auto c:qAsConst(s))
    {
        RXChar rxch(c, newLine, 0, carrier);
        newLine = false;
        engineWindow->rxBuff.addChar(rxch);
    }
}
void TestFrame::onSendCharacters(QString data, int c)
{
    sendCharacters(data, c);
}

void TestFrame::onRigModeFreq(QString, Frequency)
{

}
void TestFrame::sendCharacters(const QString &toSend, int /*carrier*/)
{
    mShowMessage(toSend, this);
}

void TestFrame::sendMode(QString mode)
{
    QString nm = "New mode set: " +  mode;
    bool newLine = true;
    for (auto c:qAsConst(nm))
    {
        RXChar rxch(c, newLine, 0, carrier);
        newLine = false;
        engineWindow->rxBuff.addChar(rxch);
    }

}

void TestFrame::closeFrame()
{

}

void TestFrame::onSetSpeeds(QString b, QString r)
{
    trace(QString("BPSK speed set to %1 RTTY speed set to %2").arg(b).arg(r));
}
