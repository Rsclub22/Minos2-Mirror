#ifndef MSGFRAME_H
#define MSGFRAME_H

#include <QFrame>

namespace Ui {
class msgFrame;
}

class msgFrame : public QFrame
{
    Q_OBJECT

public:
    explicit msgFrame(QWidget *parent = nullptr);
    ~msgFrame();

private:
    Ui::msgFrame *ui;
};

#endif // MSGFRAME_H
