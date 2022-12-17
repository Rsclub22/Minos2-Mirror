#ifndef TESTFRAME_H
#define TESTFRAME_H

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


    explicit TestFrame(QWidget *parent, QLineEdit *sendEdit, QString fname);
    ~TestFrame();

    void sendCharacters(const QString &);
    void closeFrame();


private:
    Ui::TestFrame *ui;
};

#endif // TESTFRAME_H
