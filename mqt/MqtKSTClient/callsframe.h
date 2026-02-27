#ifndef CALLSFRAME_H
#define CALLSFRAME_H

#include <QFrame>

namespace Ui {
class callsFrame;
}

class callsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit callsFrame(QWidget *parent = nullptr);
    ~callsFrame();

private:
    Ui::callsFrame *ui;
};

#endif // CALLSFRAME_H
