#ifndef LOGINFRAME_H
#define LOGINFRAME_H

#include <QFrame>

namespace Ui {
class loginFrame;
}

class loginFrame : public QFrame
{
    Q_OBJECT

public:
    explicit loginFrame(QWidget *parent = nullptr);
    ~loginFrame();

private:
    Ui::loginFrame *ui;
};

#endif // LOGINFRAME_H
