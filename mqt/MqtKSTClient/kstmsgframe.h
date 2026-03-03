#ifndef KSTMSGFRAME_H
#define KSTMSGFRAME_H

#include <QFrame>

namespace Ui {
class KSTMsgFrame;
}

class KSTMsgFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTMsgFrame(QWidget *parent = nullptr);
    ~KSTMsgFrame();

private:
    Ui::KSTMsgFrame *ui;
};

#endif // KSTMSGFRAME_H
