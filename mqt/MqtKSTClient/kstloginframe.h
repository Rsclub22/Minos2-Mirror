#ifndef KSTLOGINFRAME_H
#define KSTLOGINFRAME_H

#include <QFrame>

namespace Ui {
class KSTLoginFrame;
}

class KSTLoginFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTLoginFrame(QWidget *parent = nullptr);
    ~KSTLoginFrame();

private:
    Ui::KSTLoginFrame *ui;
};

#endif // KSTLOGINFRAME_H
