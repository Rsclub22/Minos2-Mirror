#ifndef TESTFRAME_H
#define TESTFRAME_H

#include <QTextEdit>
#include <QLineEdit>
#include <QFrame>

#include "frequency.h"

class EngineWindow;

namespace Ui {
class TestFrame;
}
class QTimer;

class TestFrame : public QFrame
{
    Q_OBJECT

public:
    QLineEdit *sendEdit = nullptr;
    QTimer *testTimer = nullptr;
    EngineWindow *engineWindow = nullptr;

    int markFrequency = 0;

    explicit TestFrame(EngineWindow *parent, QLineEdit *sendEdit, QString fname, QString name);
    ~TestFrame();

    void sendCharacters(const QString &, int c);
    void sendMode(QString);

    void closeFrame();
private slots:
    void onSetSpeeds(QString b, QString m);

    void onSendCharacters(QString, int c);
    void onRigModeFreq(QString, Frequency);

    void onTimeout();
private:
    Ui::TestFrame *ui;
    int loffset = 0;
    int toffset = 0;
    bool newLine = false;

};

#endif // TESTFRAME_H
