#ifndef TESTFRAME_H
#define TESTFRAME_H

#include "frequency.h"
#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>

namespace Ui {
class TestFrame;
}

class TestFrame : public QFrame
{
    Q_OBJECT

public:
    QLineEdit *sendEdit = nullptr;

    int carrier = 0;

    explicit TestFrame(QWidget *parent, QLineEdit *sendEdit, QString fname);
    ~TestFrame();

    void sendCharacters(const QString &, int c);
    void sendMode(QString);

    void closeFrame();
private slots:
    void onSendCharacters(QString, int c);
    void onRigModeFreq(QString, Frequency);

private:
    Ui::TestFrame *ui;
};

#endif // TESTFRAME_H
