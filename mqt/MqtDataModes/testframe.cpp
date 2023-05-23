#include "MShowMessageDlg.h"
#include "rxbuffer.h"
#include "MTrace.h"
#include "dmmainwindow.h"
#include "enginewindow.h"
#include "testframe.h"
#include "ui_testframe.h"

QStringList testData = {
"CQ CQ BARTG G0GVV SP3P G0GVV TEST",
"G0GJV DE PE1EWR PE1EWR",
"PE1EWR 599-001-001_001 K",
"G0GVV 599 020 020 020 PE1EWR",
"PE1EWR TU F/G0GJV/P QRX BARTG",
"I AM M5N REALLY",
    "CQ CQ BARTG G0GVV SP3P G0GVV TEST",
    "G0GJV DE PE1EWR PE1EWR",
    "PE1EWR 599-001-001_001 K",
    "G0GVV 599 020 020 020 PE1EWR",
    "PE1EWR TU F/G0GJV/P QRX BARTG",
    "I AM M5N REALLY",
    "CQ CQ BARTG G0GVV SP3P G0GVV TEST",
    "G0GJV DE PE1EWR PE1EWR",
    "PE1EWR 599-001-001_001 K",
    "G0GVV 599 020 020 020 PE1EWR",
    "PE1EWR TU F/G0GJV/P QRX BARTG",
    "I AM M5N REALLY",
    "CQ CQ BARTG G0GVV SP3P G0GVV TEST",
    "G0GJV DE PE1EWR PE1EWR",
    "PE1EWR 599-001-001_001 K",
    "G0GVV 599 020 020 020 PE1EWR",
    "PE1EWR TU F/G0GJV/P QRX BARTG",
    "I AM M5N REALLY",
    "CQ CQ BARTG G0GVV SP3P G0GVV TEST",
    "G0GJV DE PE1EWR PE1EWR",
    "PE1EWR 599-001-001_001 K",
    "G0GVV 599 020 020 020 PE1EWR",
    "PE1EWR TU F/G0GJV/P QRX BARTG",
    "I AM M5N REALLY"
};
// to run multiple copies - just do it!

TestFrame::TestFrame(EngineWindow *parent, QLineEdit */*sendEdit*/, QString /*fname*/, QString /*name*/) :
    QFrame(parent),
    engineWindow(parent),
    ui(new Ui::TestFrame)
{
    ui->setupUi(this);
    connect(mainWindow, &DMMainWindow::setSpeeds, this, &TestFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::setSpeeds, this, &TestFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::sendCharactersDown, this, &TestFrame::onSendCharacters);
    connect(engineWindow, &EngineWindow::rigModeFreq, this, &TestFrame::onRigModeFreq);

//    for(const auto &s:qAsConst(testData))
//    {
//        trace(QString("Response %1").arg(s));
//        bool newLine = true;
//        for (auto c:qAsConst(s))
//        {
//            RXChar rxch(c, newLine, 0, markFrequency);
//            newLine = false;
//            engineWindow->rxBuff.addChar(rxch);
//        }
//    }

    testTimer = new QTimer(this);
    connect(testTimer, &QTimer::timeout, this, &TestFrame::onTimeout);
    testTimer->start(100);
}

TestFrame::~TestFrame()
{
    delete ui;
}
void TestFrame::onTimeout()
{
    if (loffset < testData.size())
    {
        if (toffset >= testData[loffset].size())
        {
            loffset++;
            toffset = 0;
            RXChar rxch('\n', 0, markFrequency);
            engineWindow->rxBuff.addChar(rxch);
        }
        else
        {
            QChar c = testData[loffset][toffset];
            RXChar rxch(c, 0, markFrequency);
            engineWindow->rxBuff.addChar(rxch);
            toffset++;
        }
    }
    else
    {
//        static int n = 0;
//        QString s = QString::number(n);
//        n++;
//        bool newLine = true;
//        for (auto c:qAsConst(s))
//        {
//            RXChar rxch(c, newLine, 0, markFrequency);
//            newLine = false;
//            engineWindow->rxBuff.addChar(rxch);
//        }
    }
}
void TestFrame::onSendCharacters(QString data, int c)
{
    sendCharacters(data, c);
}

void TestFrame::onRigModeFreq(QString, Frequency)
{

}
void TestFrame::sendCharacters(const QString &toSend, int /*markf*/)
{
    mShowMessage(toSend, this);
}

void TestFrame::sendMode(QString mode)
{
    QString nm = "New mode set: " +  mode;
    RXChar rxchn('\n', 0, markFrequency);
    engineWindow->rxBuff.addChar(rxchn);
    for (auto c:qAsConst(nm))
    {
        RXChar rxch(c, 0, markFrequency);
        newLine = false;
        engineWindow->rxBuff.addChar(rxch);
    }

}

void TestFrame::closeFrame()
{

}

void TestFrame::onSetSpeeds(QString b, QString r)
{
    trace(QString("BPSK speed set to %1 RTTY speed set to %2").arg(b, r));
}
