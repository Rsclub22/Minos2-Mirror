#include "pccwkeyermainwindow.h"
#include "ui_pccwkeyermainwindow.h"
#include <QVBoxLayout>
#include <QTimer>


pcCwKeyerMainWindow::pcCwKeyerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pcCwKeyerMainWindow)
{
    ui->setupUi(this);

    cwKeyer = new PcCwKeyer("COM8", 20, 10, true, this);

    connect(ui->cwTextInputLineEdit, &QLineEdit::textEdited, this, &pcCwKeyerMainWindow::onTextEdited);

    // Timer to feed CW keyer every 100ms
    auto *bufferTimer = new QTimer(this);
    connect(bufferTimer, &QTimer::timeout, this, &pcCwKeyerMainWindow::checkCWBuffer);
    bufferTimer->start(100);
}

pcCwKeyerMainWindow::~pcCwKeyerMainWindow()
{
    delete ui;
}



void pcCwKeyerMainWindow::onTextEdited(const QString &text) {
    static int lastLen = 0;
    if (text.length() > lastLen) {
        QString added = text.mid(lastLen);
        pendingBuffer += added;
    } else if (text.length() < lastLen) {
        // Handle backspace or clear (optional logic)
    }
    lastLen = text.length();
}

void pcCwKeyerMainWindow::checkCWBuffer() {
    if (!pendingBuffer.isEmpty() && !cwKeyer->isBusy()) {
        QChar next = pendingBuffer[0];
        pendingBuffer.remove(0, 1);
        cwKeyer->sendText(QString(next));
    }
}
