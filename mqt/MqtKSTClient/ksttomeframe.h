#ifndef KSTTOMEFRAME_H
#define KSTTOMEFRAME_H

#include <QFrame>

namespace Ui {
class KSTTomeFrame;
}

class KSTTomeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit KSTTomeFrame(QWidget *parent = nullptr);
    ~KSTTomeFrame();

private:
    Ui::KSTTomeFrame *ui;
};

#endif // KSTTOMEFRAME_H
