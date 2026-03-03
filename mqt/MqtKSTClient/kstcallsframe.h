#ifndef KSTCALLSFRAME_H
#define KSTCALLSFRAME_H

#include <QFrame>

namespace Ui {
class KSTCallsFrame;
}

class KSTCallsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTCallsFrame(QWidget *parent = nullptr);
    ~KSTCallsFrame();

private:
    Ui::KSTCallsFrame *ui;
};

#endif // KSTCALLSFRAME_H
