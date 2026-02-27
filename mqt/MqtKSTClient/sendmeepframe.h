#ifndef SENDMEEPFRAME_H
#define SENDMEEPFRAME_H

#include <QFrame>

namespace Ui {
class sendMeepFrame;
}

class sendMeepFrame : public QFrame
{
    Q_OBJECT

public:
    explicit sendMeepFrame(QWidget *parent = nullptr);
    ~sendMeepFrame();

private:
    Ui::sendMeepFrame *ui;
};

#endif // SENDMEEPFRAME_H
