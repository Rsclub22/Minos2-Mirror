#ifndef PLANESFRAME_H
#define PLANESFRAME_H

#include <QFrame>

namespace Ui {
class planesFrame;
}

class planesFrame : public QFrame
{
    Q_OBJECT

public:
    explicit planesFrame(QWidget *parent = nullptr);
    ~planesFrame();

private:
    Ui::planesFrame *ui;
};

#endif // PLANESFRAME_H
